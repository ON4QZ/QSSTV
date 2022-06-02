/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   http://users.telenet.be/on4qz                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "cameradialog.h"
#include "ui_cameradialog.h"

#include "appglobal.h"
#include <libv4l2.h>
#include "imagesettings.h"
#include "videocapture.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <cstring>


#include <QMessageBox>
#include <QPalette>
//#include <QtWidgets>
//#include <QDebug>
#define NUMRES 5

//standard resolutions
int standardResolution[NUMRES][2]=
{
  //  {1920,1080},
  {1280,720},
  {800,600},
  {640,480},
  {320,240},
  {160,120}
};


cameraDialog::cameraDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::cameraDialog)
{
  ui->setupUi(this);
  cameraActive=false;
  videoCapturePtr=NULL;
  listCameraDevices();
  if(cameraList.count()==0)
    {
      return;
    }
  connect(ui->settingsButton,SIGNAL(clicked()),SLOT(slotSettings()));
  ui->devicesComboBox->setCurrentIndex(0);
  imageSettings settingsDialog(cameraList.at(0).deviceName);

  videoCapturePtr=new videoCapture;
  connect(ui->devicesComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotDeviceChanged(int)));
  connect(ui->formatsComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotFormatChanged(int)));
  connect(ui->sizeComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotSizeChanged(int)));
  timerID=0;

}

cameraDialog::~cameraDialog()
{
  delete ui;
}

int cameraDialog::exec()
{
  if(!restartCapturing(true))
    {
      QMessageBox::warning(this,"Capturing","Unable to start capturing");
      return QDialog::Rejected;
    }
  int result;
  addToLog("cameracontrol exec",LOGCAM);
  result=QDialog::exec();
  deactivateTimer();
  videoCapturePtr->stopStreaming();
  videoCapturePtr->close();
  if(result==QDialog::Accepted) return true;
  return false;
}


void cameraDialog::deactivateTimer()
{
  if(timerID) killTimer(timerID);
  timerID=0;
}

void cameraDialog::timerEvent(QTimerEvent *)
{
  int ret;
  ret=videoCapturePtr->getFrame();
  if(ret>0)
    {

      ui->viewFinder->openImage(*videoCapturePtr->getImage());
    }
  else if(ret==0)
    {
      return;
    }
  else
    {
      deactivateTimer();
      videoCapturePtr->stopStreaming();
      videoCapturePtr->close();
      QMessageBox::critical(this,"Capture error",videoCapturePtr->getErrorString());
    }
}



QImage *cameraDialog::getImage()
{
  return videoCapturePtr->getImage();
}




void cameraDialog::slotSettings()
{
  imageSettings settingsDialog(cameraList.at(ui->devicesComboBox->currentIndex()).deviceName);
  settingsDialog.exec();
}

void cameraDialog::listCameraDevices()
{
  int i;
  cameraList.clear();
  QDir devDir("/dev");
  QStringList devList;
  devDir.setFilter(QDir::System| QDir::NoSymLinks);
  devDir.setSorting(QDir::Name);
  devDir.setNameFilters(QStringList("video*"));
  devList=devDir.entryList();
  getCameraInfo(devList);
  for(i=0;i<cameraList.count();i++)
    {
      ui->devicesComboBox->addItem(cameraList.at(i).deviceDescription);
    }
  if(cameraList.count()>0) setupFormatComboBox(cameraList.at(0));
}


void cameraDialog::setupFormatComboBox(scameraDevice cd)
{
  int i;
  ui->formatsComboBox->blockSignals(true);
  ui->formatsComboBox->clear();
  for(i=0;i<cd.formats.count();i++)
    {
      ui->formatsComboBox->addItem(cd.formats.at(i).description);
    }
  ui->formatsComboBox->setCurrentIndex(cd.formatIdx);
  ui->formatsComboBox->blockSignals(false);
  setupSizeComboBox(cd.formats.at(cd.formatIdx));
}

void cameraDialog::setupSizeComboBox(sformats frmat)
{
  int i;
  ui->sizeComboBox->blockSignals(true);
  ui->sizeComboBox->clear();
  for(i=0;i<frmat.cameraSizes.count();i++)
    {
      ui->sizeComboBox->addItem(frmat.cameraSizes.at(i).description);
    }
  ui->sizeComboBox->setCurrentIndex(frmat.sizeIdx);
  ui->sizeComboBox->blockSignals(false);
}


void cameraDialog::getCameraInfo(QStringList devList)
{
  int fd;
  int i;
  bool ok=true;
  QString camDev;

  QList <sformats> formats;

  for(i=0;i<devList.count();i++)
    {
      ok=true;
      struct v4l2_capability cap;
      memset(&cap, 0, sizeof(cap));
      camDev=QString("/dev/")+devList.at(i);
      fd = v4l2_open(camDev.toLatin1().data(), O_RDWR, 0);
      if(fd < 0)
        {
          QString msg=QString("Unable to open file %1\n%2").arg(camDev).arg(strerror(errno));
          QMessageBox::warning(this,"v4l2ucp: Unable to open file", msg, "OK");
          continue;
        }

      if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
        {
          QString msg=QString("%1 is not a V4L2 device").arg(camDev);
          QMessageBox::warning(this, "Camera selection error", msg, "OK");
          ok=false;
        }
      formats=getFormatList(fd);
      if(ok)
        {
          cameraList.append(scameraDevice(camDev,(const char *)cap.card,(const char *)cap.driver,(const char *)cap.bus_info,formats));
        }
      v4l2_close(fd);
    }
}

QString cameraDialog::pixelFormatStr(int pixelFormat)
{
  QString t;
  t=pixelFormat&0xFF;
  t+=(pixelFormat>>8)&0xFF;
  t+=(pixelFormat>>16)&0xFF;
  t+=(pixelFormat>>24)&0xFF;
  return t;
}

QList<sformats> cameraDialog::getFormatList(int fd)
{
  int j,ret;
  unsigned int resx,resy;
  QList<sformats> formatsList;
  v4l2_frmsizeenum frm;
  struct v4l2_fmtdesc fmt;
  QList<scameraSizes> scsList;

  int i = 0;
  do
    {
      scsList.clear();
      memset(&fmt, 0, sizeof fmt);
      fmt.index = i;
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if ((ret = v4l2_ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) < 0)
        break;
      else
        {
          frm.index=0;
          frm.pixel_format=fmt.pixelformat;
          while(v4l2_ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frm) >=0)
            {
              if(frm.type==V4L2_FRMSIZE_TYPE_DISCRETE)
                {
                  scsList.append(scameraSizes(frm.discrete.width,frm.discrete.height,QString("%1x%2").arg(frm.discrete.width).arg(frm.discrete.height)));
                }
              else // we have a stepwise resolution
                {
                  // scsList.append(scameraSizes(frm.stepwise.max_width,frm.stepwise.max_height,QString("%1x%2").arg(frm.stepwise.max_width).arg(frm.stepwise.max_height)));
                  for(j=0;j<NUMRES;j++)
                    {
                      resx=standardResolution[j][0];
                      resy=standardResolution[j][1];
                      if((resx<=frm.stepwise.max_width) && (resy<=frm.stepwise.max_height) && (resx>=frm.stepwise.min_width) && (resy>=frm.stepwise.min_height))
                        scsList.append(scameraSizes(resx,resy,QString("%1x%2").arg(resx).arg(resy)));
                    }
                }

              frm.index++;
            }
          formatsList.append(sformats(fmt.pixelformat,pixelFormatStr(fmt.pixelformat),scsList));
        }
      i++;
    }
  while (ret != EINVAL);
  return formatsList;
}


void cameraDialog::slotDeviceChanged(int idx)
{
  setupFormatComboBox(cameraList.at(idx));
  slotFormatChanged(cameraList.at(idx).formatIdx);
}

void cameraDialog::slotFormatChanged(int idx)
{
  setupSizeComboBox(cameraList.at(ui->devicesComboBox->currentIndex()).formats.at(idx));
  slotSizeChanged(cameraList.at(ui->devicesComboBox->currentIndex()).formats.at(idx).sizeIdx);
  cameraList[(ui->devicesComboBox->currentIndex())].formatIdx=idx;

}

void cameraDialog::slotSizeChanged(int idx)
{
  cameraList[ui->devicesComboBox->currentIndex()].formats[ui->formatsComboBox->currentIndex()].sizeIdx=idx;
  restartCapturing();
}

bool cameraDialog::restartCapturing(bool first)
{
  int ret;
  if(!videoCapturePtr) return false;
  if(!first)
    {
      deactivateTimer();
      videoCapturePtr->stopStreaming();
      videoCapturePtr->close();
    }
  ret=videoCapturePtr->open(cameraList.at(ui->devicesComboBox->currentIndex()).deviceName);
  if(ret<0)
    {
      return false;
    }
  if(!videoCapturePtr->init(cameraList.at(ui->devicesComboBox->currentIndex()).formats.at(ui->formatsComboBox->currentIndex()).format,
                            cameraList.at(ui->devicesComboBox->currentIndex()).formats.at(ui->formatsComboBox->currentIndex()).cameraSizes.at(ui->sizeComboBox->currentIndex()).width,
                            cameraList.at(ui->devicesComboBox->currentIndex()).formats.at(ui->formatsComboBox->currentIndex()).cameraSizes.at(ui->sizeComboBox->currentIndex()).height))
    {
      return false;
    }
  cameraActive=true;
  videoCapturePtr->startSnapshots();
  timerID=startTimer(50);
  return true;
}


