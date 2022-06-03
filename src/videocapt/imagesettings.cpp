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
*                                                                         *
*                                                                         *
*                                                                         *
* Part of this software has been copied from examples of the Qt Toolkit.  *
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "imagesettings.h"
#include "ui_imagesettings.h"
#include "v4l2control.h"

#include <QComboBox>
#include <QDebug>
#include <libv4l2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <QMessageBox>
#include <QLabel>
#include <QScrollArea>


imageSettings::imageSettings(QString cameraDevice, QWidget *parent) :   QDialog(parent)
{
  camDev=cameraDevice.toLatin1();
  ui= new Ui::imageSettingsUi;
  ui->setupUi(this);
  gridLayout=NULL;
  loadCapabilities();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(false);
  ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
}

imageSettings::~imageSettings()
{
  delete ui;
}



bool imageSettings::loadCapabilities()
{
  int fd;
  struct v4l2_capability cap;
  struct v4l2_queryctrl ctrl;
  row=0;
  fd = v4l2_open(camDev, O_RDWR, 0);
  if(fd < 0)
    {
      QString msg=QString("Unable to open file %1\n%2").arg(camDev.constData()).arg(strerror(errno));
      QMessageBox::warning(NULL, "v4l2ucp: Unable to open file", msg, "OK");
      return false;
    }

  if(v4l2_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
      QString msg;
      msg=QString("%1 is not a V4L2 device").arg(camDev.constData());
      QMessageBox::warning(NULL, "v4l2ucp: Not a V4L2 device", msg, "OK");
      return false;
    }
  ui->driverLabel->setText((const char *)cap.driver);
  ui->cardLabel->setText((const char *)cap.card);
  ui->busLabel->setText((const char *)cap.bus_info);
  ui->deviceLabel->setText(camDev.constData());

#ifdef V4L2_CTRL_FLAG_NEXT_CTRL
  /* Try the extended control API first */
  ctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
  if(v4l2_ioctl (fd, VIDIOC_QUERYCTRL, &ctrl)==0)
    {
      do
        {
          addControl(ctrl, fd);
          ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
      while(v4l2_ioctl (fd, VIDIOC_QUERYCTRL, &ctrl)==0);
    }
  else
#endif
    {
      /* Fall back on the standard API */
      /* Check all the standard controls */
      for(int i=V4L2_CID_BASE; i<V4L2_CID_LASTP1; i++) {
          ctrl.id = i;
          if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
              addControl(ctrl, fd);
            }
        }

      /* Check any custom controls */
      for(int i=V4L2_CID_PRIVATE_BASE; ; i++) {
          ctrl.id = i;
          if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == 0) {
              addControl(ctrl, fd);
            } else {
              break;
            }
        }
    }
  return true;
}

void imageSettings::addControl(struct v4l2_queryctrl &ctrl, int fd)
{
  QWidget *w = NULL;

  if(ctrl.flags & V4L2_CTRL_FLAG_DISABLED) return;
  if((ctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS) && (gridLayout==NULL))
     {
      addNewTab("Controls");
    }

  switch(ctrl.type)
    {
    case V4L2_CTRL_TYPE_INTEGER:
      w = new V4L2IntegerControl(fd, ctrl, grid);
      break;
    case V4L2_CTRL_TYPE_BOOLEAN:
      w = new V4L2BooleanControl(fd, ctrl, grid);
      break;
    case V4L2_CTRL_TYPE_MENU:
      w = new V4L2MenuControl(fd, ctrl, grid);
      break;
    case V4L2_CTRL_TYPE_BUTTON:
      w = new V4L2ButtonControl(fd, ctrl, grid);
      break;

    case V4L2_CTRL_TYPE_CTRL_CLASS:
      {
        addNewTab(QString((const char *)ctrl.name));
        return;
      }
    case V4L2_CTRL_TYPE_INTEGER64:
    default:
      break;
    }

  if(!w) {
      new QLabel("Unknown control", grid);
      new QLabel(grid);
      new QLabel(grid);
      return;
    }
  QLabel *l = new QLabel((const char *)ctrl.name, grid);
  gridLayout->addWidget(l,row,0);
  gridLayout->addWidget(w,row,1);
  if(ctrl.flags & V4L2_CTRL_FLAG_GRABBED)
    {
      w->setEnabled(false);
    }
  else
    {
      w->setEnabled(true);
    }

  QPushButton *pb;
  pb = new QPushButton("Update", grid);
  gridLayout->addWidget(pb,row,2);
  QObject::connect( pb, SIGNAL(clicked()), w, SLOT(updateStatus()) );

  if(ctrl.type == V4L2_CTRL_TYPE_BUTTON)
    {
      l = new QLabel(grid);
      gridLayout->addWidget(l,row,3);
    }
  else
    {
      pb = new QPushButton("Reset", grid);
      gridLayout->addWidget(pb,row,3);
      QObject::connect(pb, SIGNAL(clicked()), w, SLOT(resetToDefault()) );

    }
  row++;
}

void imageSettings::addNewTab(QString tabName)
{
  QVBoxLayout *vLayout;
  QWidget *tab=new QWidget();
  vLayout = new QVBoxLayout(tab);
  QScrollArea *scrollArea = new QScrollArea(tab);
  scrollArea->setWidgetResizable(true);
  grid = new QWidget();
  grid->setGeometry(QRect(0, 0, 592, 100));
  scrollArea->setWidget(grid);
  vLayout->addWidget(scrollArea);
  ui->tabWidget->addTab(tab, tabName);
  gridLayout = new QGridLayout();
  grid->setLayout(gridLayout);
  gridLayout->setSpacing(0);
  gridLayout->setMargin(1);
  gridLayout->setContentsMargins(0, 0, 0, 0);
}

void imageSettings::showEvent(QShowEvent * event)
{
  ui->buttonBox->button(QDialogButtonBox::Ok)->setDefault(false);
  ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);
  QDialog::showEvent(event);
}
