/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   https://www.qsl.net/o/on4qz                                           *
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
*   This software has been partially copied from the program:             *
*    qv4l2: a control panel controlling v4l2 devices.                     *
*                                                                         *
*  Copyright (C) 2006 Hans Verkuil <hverkuil@xs4all.nl>                   *                                                                       *
***************************************************************************/


#include "v4l2control.h"

#include <sys/ioctl.h>
#include <cerrno>
#include <cstring>
//#include <libv4l2.h>
#ifndef __Linux__
#include <linux/types.h>          /* for videodev2.h */
#endif /* __Linux__ */
#include <linux/videodev2.h>

#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QMessageBox>
#include <QLineEdit>
#include <QDebug>

V4L2Control::V4L2Control(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
  QWidget(parent), cid(ctrl.id), default_value(ctrl.default_value)
{
  this->fd = fd;
  strncpy(name, (const char *)ctrl.name, sizeof(name)-1);
  name[sizeof(name)-1] = '\0';
  layout.setSpacing(1);
  layout.setContentsMargins(0,0,0,0);
  this->setLayout(&layout);
}



void V4L2Control::updateHardware()
{
  struct v4l2_control c;
  c.id = cid;
  c.value = getValue();
  if(v4l2_ioctl(fd, VIDIOC_S_CTRL, &c) == -1)
    {
      QString msg;
      msg=QString("Unable to set %1\n%2").arg(name).arg(strerror(errno));
      (void)QMessageBox::warning(this, "Unable to set control", msg,
                                 QMessageBox::Ok, QMessageBox::Ok);
    }
  updateStatus();
}

void V4L2Control::updateStatus()
{
  struct v4l2_control c;
  c.id = cid;
  if(v4l2_ioctl(fd, VIDIOC_G_CTRL, &c) == -1)
    {
      QString msg;
      msg=QString("Unable to get %1\n%2").arg(name).arg(strerror(errno));
      (void)QMessageBox::warning(this, "Unable to get control", msg,
                                 QMessageBox::Ok, QMessageBox::Ok);
    }
  else
    {
      if(c.value != getValue())
        setValue(c.value);
    }
  struct v4l2_queryctrl ctrl;
  ctrl.id = cid;
  if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == -1)
    {
      QString msg;
      msg=QString("Unable to get %1\n%2").arg(name).arg(strerror(errno));
      (void)QMessageBox::warning(this, "Unable to get control status", msg,
                                 QMessageBox::Ok, QMessageBox::Ok);
    }
  else
    {
      setEnabled((ctrl.flags &( V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_GRABBED)) == 0);
    }
}

void V4L2Control::resetToDefault()
{
  if(isEnabled())
    {
      setValue(default_value);
      updateHardware();
    }
}

/*
 * V4L2IntegerControl
 */
V4L2IntegerControl::V4L2IntegerControl
(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
  V4L2Control(fd, ctrl, parent),
  minimum(ctrl.minimum), maximum(ctrl.maximum), step(ctrl.step)
{
  int pageStep = (maximum-minimum)/10;
  if(step > pageStep)
    pageStep = step;
  sl = new QSlider(Qt::Horizontal, this);
  sl->setMinimum(minimum);
  sl->setMaximum(maximum);
  sl->setFocusPolicy(Qt::StrongFocus);
  sl->setPageStep(pageStep);
  sl->setValue(default_value);
  sl->setVisible(true);
  this->layout.addWidget(sl);

  QString defStr;
  defStr.setNum(default_value);
  le = new QLineEdit(this);
  le->setText(defStr);
  le->setValidator(new QIntValidator(minimum, maximum, this));
  this->layout.addWidget(le);

  QObject::connect( sl, SIGNAL(valueChanged(int)),this, SLOT(SetValueFromSlider()) );
  QObject::connect( sl, SIGNAL(sliderReleased()), this, SLOT(SetValueFromSlider()) );
  QObject::connect( le, SIGNAL(returnPressed()),  this, SLOT(SetValueFromText()) );
  updateStatus();
}

void V4L2IntegerControl::setValue(int val)
{
  if(val < minimum) val = minimum;
  if(val > maximum) val = maximum;
  if(step > 1)
    {
      int mod = (val-minimum)%step;
      if(mod > step/2)
        {
          val += step-mod;
        }
      else
        {
          val -= mod;
        }
    }
  QString str;
  str.setNum(val);
  le->setText(str);

  /* FIXME: find clean solution to prevent infinite loop */
  sl->blockSignals(true);
  sl->setValue(val);
  sl->blockSignals(false);
}

int V4L2IntegerControl::getValue()
{
  return sl->value();
}

void V4L2IntegerControl::SetValueFromSlider()
{
  setValue(sl->value());
  updateHardware();
}

void V4L2IntegerControl::SetValueFromText()
{
  if(le->hasAcceptableInput())
    {
      setValue(le->text().toInt());
      updateHardware();
    }
  else
    {
      SetValueFromSlider();
    }
}

/*
 * V4L2BooleanControl
 */
V4L2BooleanControl::V4L2BooleanControl
(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
  V4L2Control(fd, ctrl, parent),
  cb(new QCheckBox(this))
{
  this->layout.addWidget(cb);
  QObject::connect( cb, SIGNAL(clicked()), this, SLOT(updateHardware()) );
  updateStatus();
}

void V4L2BooleanControl::setValue(int val)
{
  cb->setChecked(val != 0);
}

int V4L2BooleanControl::getValue()
{
  return cb->isChecked();
}

/*
 * V4L2MenuControl
 */
V4L2MenuControl::V4L2MenuControl(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :  V4L2Control(fd, ctrl, parent)
{
  int i,j;
  int defIdx=0;
  cb = new QComboBox(this);
  this->layout.addWidget(cb);

  for(i=ctrl.minimum,j=0; i<=ctrl.maximum; i++,j++)
    {
      struct v4l2_querymenu qm;
      qm.id = ctrl.id;
      qm.index = i;

      if(v4l2_ioctl(fd, VIDIOC_QUERYMENU, &qm) == 0)
        {
          qDebug() << QString((const char *)ctrl.name) <<  QString((const char *)qm.name);
          cb->insertItem(i, (const char *)qm.name);
          indexValueArray[j]=i;
          if(i==default_value)
            {
              defIdx=j;
            }
        }
      else
        {
          cb->insertItem(i,"Unsupported");
        }

    }
//  cb->setCurrentIndex(defIdx);
  setValue(defIdx);
  QObject::connect( cb, SIGNAL(activated(int)),
                    this, SLOT(menuActivated(int)) );
  updateStatus();
  updateHardware();
}

void V4L2MenuControl::setValue(int val)
{
  cb->setCurrentIndex(val);
}

int V4L2MenuControl::getValue()
{
  return indexValueArray[cb->currentIndex()];
}

void V4L2MenuControl::menuActivated(int val)
{
  setValue(val);
  updateHardware();
}

/*
 * V4L2ButtonControl
 */
V4L2ButtonControl::V4L2ButtonControl
(int fd, const struct v4l2_queryctrl &ctrl, QWidget *parent) :
  V4L2Control(fd, ctrl, parent)
{
  QPushButton *pb = new QPushButton((const char *)ctrl.name, this);
  this->layout.addWidget(pb);
  QObject::connect( pb, SIGNAL(clicked()), this, SLOT(updateHardware()) );
}

void V4L2ButtonControl::updateStatus()
{
  struct v4l2_queryctrl ctrl;
  ctrl.id = cid;
  if(v4l2_ioctl(fd, VIDIOC_QUERYCTRL, &ctrl) == -1) {
      QString msg;
      msg=QString("Unable to get the status of %1\n%2").arg(name).arg(strerror(errno));
      (void)QMessageBox::warning(this, "Unable to get control status", msg,
                                 QMessageBox::Ok, QMessageBox::Ok);
    } else {
      setEnabled((ctrl.flags &( V4L2_CTRL_FLAG_DISABLED | V4L2_CTRL_FLAG_GRABBED)) == 0);
    }
}

void V4L2ButtonControl::resetToDefault()
{
}
