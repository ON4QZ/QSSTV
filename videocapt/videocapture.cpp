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
*   This software contains parts of the following softwares               *
*      videoCapture.cpp -- Kapture                                        *
*                                                                         *
*      Copyright (C) 2006-2009                                            *
*          Detlev Casanova (detlev.casanova@gmail.com)                    *
*                                                                         *
*  qv4l2: a control panel controlling v4l2 devices.                       *
*                                                                         *
*  Copyright (C) 2006 Hans Verkuil <hverkuil@xs4all.nl>                   *                                                                       *
***************************************************************************/

#include "videocapture.h"
#include "configparams.h"
#include "appglobal.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <libv4l2.h>
#include <libv4lconvert.h>

#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QImage>
#include <QPixmap>

#include <QLabel>
#include <QSize>



videoCapture::videoCapture()
{
  dev = 0;
  opened = false;
  allocated = false;
  localImage=nullptr;
  numBuffers=2;

}

videoCapture::~videoCapture()
{
  if(localImage!=nullptr) delete localImage;
  close();
}

void videoCapture::close()
{
  if(!opened) return;

  v4l2_close(dev);
  opened = false;
  allocated = false;
}

bool videoCapture::open(QString videoDev)
{
  videoDevice=videoDev;
  if (opened) return true;
  addToLog("opening Videocapture device",LOGCAM);

  dev = v4l2_open(videoDevice.toLatin1().data(), O_RDWR);
  if (dev < 0)
    {
      addToLog(QString("Error opening %1, %2").arg(videoDevice).arg(errno),LOGCAM);
      return false;
    }
  //  dumpCaps(cap);
  opened = true;
  return opened;
}


bool videoCapture::init(int pixelFormat,int width,int height)
{

  getFormat(srcFmt);
  srcFmt.fmt.pix.pixelformat=static_cast<uint>(pixelFormat);
  srcFmt.fmt.pix.width=static_cast<uint>(width);
  srcFmt.fmt.pix.height=static_cast<uint>(height);
  //  srcFmt.fmt.pix.width=320;
  //  srcFmt.fmt.pix.height=240;
  setFormat(srcFmt);
  dstFmt=srcFmt;
  qFmt=checkConversionNeeded();
  if(qFmt==QImage::Format_Invalid)
    {
      needsConversion=true;
      convertData = v4lconvert_create(dev);
      dstFmt.fmt.pix.pixelformat=V4L2_PIX_FMT_RGB24;
      qFmt=QImage::Format_RGB888;
    }
  else
    {
      needsConversion=false;
    }

  if(needsConversion)
    {
      v4lconvert_try_format(convertData, &dstFmt, &srcFmt);
      getFormat(srcFmt); //restore srcFmt
    }

  if(localImage) delete localImage;
  localImage=new QImage( static_cast<int>(dstFmt.fmt.pix.width),static_cast<int>(dstFmt.fmt.pix.height),qFmt);
  if (!allocated)
    {
      if(allocateBuffers(numBuffers)>=0)
        {
          allocated=true;
        }
      else
        {
          return false;
        }
    }
  else
    {
      allocateBuffers(0); // deallocate buffers
      if(allocateBuffers(numBuffers)>=0)
        {
          allocated=true;
        }
      else
        {
          return false;
        }
    }
  return true;
}

int videoCapture::allocateBuffers(uint numBufs)
{
  int ret;
  memset(&rb, 0, sizeof rb);
  rb.count = static_cast<uint>(numBufs);
  rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  rb.memory = V4L2_MEMORY_MMAP;

  ret = v4l2_ioctl(dev, VIDIOC_REQBUFS, &rb);
  if (ret < 0)
    {
      addToLog(QString("Unable to allocate buffers for %1, %2").arg(videoDevice).arg(errno),LOGCAM);
    }
  return ret;
}


bool videoCapture::setFormat(v4l2_format &fmt)
{
  addToLog("setFormat",LOGCAM);
  //  memset(&fmt, 0, sizeof fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  //  fmt.fmt.pix.width = width;
  //  fmt.fmt.pix.height = height;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;
  //  fmt.fmt.pix.pixelformat = pixelformat;
  //  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  //  fmt.fmt.pix.pixelformat =V4L2_PIX_FMT_RGB24;
  fmt.fmt.pix.bytesperline=0;
  if (v4l2_ioctl(dev, VIDIOC_S_FMT, &fmt) < 0)
    {
      addToLog(QString("Error while setting format , %1").arg(strerror(errno)),LOGCAM);
      return false;
    }
  getFormat(fmt);
  return true;
}

bool videoCapture::getFormat(v4l2_format &fmt)
{
  addToLog("getFormat",LOGCAM);
  memset(&fmt, 0, sizeof fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(dev, VIDIOC_G_FMT, &fmt) < 0)
    {
      addToLog(QString("Error while getting format , %1").arg(errno),LOGCAM);
      return false;
    }
  return true;
}



int videoCapture::getFrame()
{
  int ret = 0;
  // Dequeue a buffer.
  errorString.clear();

  ret = v4l2_ioctl(dev, VIDIOC_DQBUF, &buf);
  //addToLog(QString("Dequeue buffer %1").arg(buf.index),LOGCAM);
  if(ret==EAGAIN)
    {
      return 0;
    }
  else if (ret < 0)
    {
      addToLog(QString("Unable to dequeue buffer , %1").arg(strerror(errno)),LOGCAM);
      errorString=QString("Unable to dequeue buffer , %1").arg(strerror(errno));
      return -1;
    }

  convert(mem[buf.index]);

  // Requeue the buffer.
  ret = v4l2_ioctl(dev, VIDIOC_QBUF, &buf);
  //addToLog(QString("Requeue buffer %1").arg(buf.index),LOGCAM);
  if (ret < 0)
    {
      addToLog(QString("Unable to requeue buffer %1").arg(errno),LOGCAM);
      return -1;
    }
  return 1;
}



bool videoCapture::convert(unsigned char *src)
{
  int result=0;
  //  if (localImage!=NULL) delete localImage;
  //  localImage=new QImage( dstFmt.fmt.pix.width,dstFmt.fmt.pix.height,qFmt);
  if(needsConversion)
    {
      result=v4lconvert_convert(convertData,&srcFmt,&dstFmt,
                                src,static_cast<int>(srcFmt.fmt.pix.sizeimage),
                                localImage->bits(),static_cast<int>(dstFmt.fmt.pix.sizeimage));
    }
  else
    {
#if (QT_VERSION < QT_VERSION_CHECK(5,10,0))
      memcpy(localImage->bits(),src,static_cast<uint>(localImage->byteCount()));
#else
      memcpy(localImage->bits(),src,static_cast<uint>(localImage->sizeInBytes()));
#endif
    }
  if (result<0) return false;
  return true;
}


int videoCapture::currentWidth(v4l2_format fmt) const
{
  return (static_cast<int>(fmt.fmt.pix.width));
}

int videoCapture::currentHeight(v4l2_format fmt) const
{
  return (static_cast<int>(fmt.fmt.pix.height));
}

int videoCapture::currentPixelFormat(v4l2_format fmt) const
{
  return (static_cast<int>( fmt.fmt.pix.pixelformat));
}






bool videoCapture::captureStop()
{
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int ret;

  if(!streaming) return false;
  ret = v4l2_ioctl(dev, VIDIOC_STREAMOFF, &type);
  if (ret < 0)
    {
      addToLog(QString("Unable  to stop capture, %1").arg(errno),LOGCAM);
      return false;
    }

  streaming = false;
  return true;
}

bool videoCapture::captureStart()
{
  uint i;
  int ret;
  if (!opened) return false;
  addToLog("captureStart",LOGCAM);

  //Allocate buffers

  if(!mmapped)
    {

      // Map the buffers.
      memset(&buf, 0, sizeof buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      for (i = 0; i < numBuffers; i++)
        {
          buf.index = static_cast<uint>(i);
          ret = v4l2_ioctl(dev, VIDIOC_QUERYBUF, &buf);
          if (ret < 0)
            {
              addToLog(QString("Unable to query buffer %1").arg(ret),LOGCAM);
              return false;
            }
          mem[i] = static_cast<uchar *>(v4l2_mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, dev, buf.m.offset));
          if (mem[i] == MAP_FAILED)
            {
              addToLog(QString("Unable to map buffers %1").arg(ret),LOGCAM);
              return false;
            }
          bufLength = buf.length;
          mmapped = true;
        }

      // Queue the buffers

      for (i = 0; i < numBuffers; i++)
        {
          buf.index = i;
          ret = v4l2_ioctl(dev, VIDIOC_QBUF, &buf);
          if (ret < 0)
            {
              addToLog(QString("Unable to queue buffer %1").arg(errno),LOGCAM);
              return false;
            }
        }
    }

  // Start streaming.
  ret = v4l2_ioctl(dev, VIDIOC_STREAMON, &buf.type);
  if (ret < 0)
    {
      addToLog(QString("Unable to start capture %1").arg(errno),LOGCAM);
      return false;
    }
  streaming = true;
  return true;
}

bool videoCapture::stopStreaming()
{
  uint i;
  bool ok=true;
  if(!streaming) return false;
  for(i=0;i<numBuffers;i++)
    {
      if (v4l2_munmap(mem[i], bufLength) == -1)
        {
          addToLog(QString("videoCapture::stopStreaming : munmap %1 failed. errno = %2").arg(i).arg(errno),LOGCAM);
          ok=false;
        }

    }
  if(ok) mmapped = false;

  if(captureStop())
    {
      streaming = false;
      addToLog(" * Succesful Stopped",LOGCAM);
    }
  return true;
}



bool videoCapture::startSnapshots()
{
  if(!opened) return false;
  if(!captureStart()) return false;
  return true;
}

enum QImage::Format videoCapture::checkConversionNeeded ()
{
  QImage::Format qfmt;
  switch(srcFmt.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_BGR32:   qfmt=QImage::Format_RGB32; break;
    case V4L2_PIX_FMT_RGB24:   qfmt=QImage::Format_RGB888 ; break;
    case V4L2_PIX_FMT_RGB565:  qfmt=QImage::Format_RGB16 ; break;
    case V4L2_PIX_FMT_RGB555:  qfmt=QImage::Format_RGB555 ; break;
    case V4L2_PIX_FMT_RGB444:  qfmt=QImage::Format_RGB444 ; break;
    default: qfmt=QImage::Format_Invalid;
    }
  return qfmt;
}


void videoCapture::dumpCaps(v4l2_capability &cap)
{
  if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) addToLog("The device supports the Video Capture interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) addToLog("The device supports the Video Output interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY) addToLog("The device supports the Video Overlay interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_VBI_CAPTURE) addToLog("The device supports the Raw VBI Capture interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_VBI_OUTPUT) addToLog("The device supports the Raw VBI Output interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE) addToLog("The device supports the Sliced VBI Capture interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT) addToLog("The device supports the Sliced VBI Output interface",LOGCAM);
  if (cap.capabilities & V4L2_CAP_RDS_CAPTURE) addToLog("[to be defined]",LOGCAM);
#ifdef V4L2_CAP_VIDEO_OUTPUT_OVERLAY
  if (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT_OVERLAY) addToLog("The device supports the Video Output Overlay (OSD) interface",LOGCAM);
#endif
  if (cap.capabilities & V4L2_CAP_TUNER) addToLog("The device has some sort of tuner or modulator",LOGCAM);
  if (cap.capabilities & V4L2_CAP_AUDIO) addToLog("The device has audio inputs or outputs.",LOGCAM);
  if (cap.capabilities & V4L2_CAP_READWRITE) addToLog("The device supports the read() and/or write() I/O methods.",LOGCAM);
  if (cap.capabilities & V4L2_CAP_ASYNCIO) addToLog("The device supports the asynchronous I/O methods.",LOGCAM);
  if (cap.capabilities & V4L2_CAP_STREAMING) addToLog("The device supports the streaming I/O method.",LOGCAM);
}



