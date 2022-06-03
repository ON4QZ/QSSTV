/***************************************************************************
 *   Copyright (C) 2008 - Giuseppe Cigala                                  *
 *   g_cigala@virgilio.it                                                  *
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


#include "vumeter.h"
#include "configparams.h"

#define BW 5
#define SG 100
#define LG 500
#define OFFSET 50



vuMeter::vuMeter(QWidget *parent) : QWidget(parent)
{
  colBack = QColor(50, 50, 255);
  colValue = Qt::white;
  colHigh = Qt::red;
  colMid =  Qt::green;
  colLow =  Qt::blue;
  min = 0;
  max = 100;
  val =9;
  prevValue = 0;
  horizontal=false;
  divisions=20;
  labelText="V";
}

void vuMeter::setLabelText(QString t)
{
  labelText=t;
}


void vuMeter::paintEvent(QPaintEvent *)
{
  if(width()>height()) horizontal=true;
  else horizontal=false;
  if (horizontal)
    {
      w=LG;
      h=SG;
      rw=5;
      rh=30; // rect rounding
    }
  else
    {
      w=SG;
      h=LG;
      rw=30;
      rh=5; // rect rounding
    }
  if(!slowCPU)
    {
      paintBorder();
    }
  paintBar();
}

void vuMeter::paintBorder()
{


  QLinearGradient linGrad;
  QLinearGradient linGrad1;
  QRectF border1;
  QRectF rct;

  QPainter painter(this);
  QColor light = Qt::white;
  QColor dark = colBack.darker(255);
  painter.setPen(QPen(colBack, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

  painter.setRenderHint(QPainter::Antialiasing);

  if (horizontal)
    {
      linGrad.setStart(250,BW); linGrad.setFinalStop(250, 2*BW);
      linGrad1.setStart(1,SG-3*BW); linGrad1.setFinalStop(1, SG-BW);
      border1=QRectF(5, 20, w-2*BW, h-5*BW);
      rct=QRectF(2*BW,h/2-10, 20, 25);
    }
  else
    {
      linGrad.setStart(BW,1); linGrad.setFinalStop(2*BW, 1);
      linGrad1.setStart(SG-3*BW,1); linGrad1.setFinalStop(SG-BW, 1);
      border1=QRectF(20, 5, w-5*BW, h-2*BW);
      rct=QRectF(w/2-10, h-4*BW-10, 20, 25);
    }
  painter.setWindow(0, 0, w, h);
  linGrad.setColorAt(0, light);
  linGrad.setColorAt(1, colBack);
  linGrad.setSpread(QGradient::PadSpread);
  painter.setBrush(linGrad);
  QRectF border(5, 5, w-2*BW, h-2*BW);
  painter.drawRoundedRect(border, rw,rh);
  linGrad1.setColorAt(0, colBack);
  linGrad1.setColorAt(1, dark);
  linGrad1.setSpread(QGradient::PadSpread);
  painter.setBrush(linGrad1);
  painter.drawRoundedRect(border1, rw,rh);


  //paint label

  painter.setPen(QPen(colValue, 2));
  QFont valFont("Arial", 24, QFont::Bold);
  painter.setFont(valFont);
  painter.drawText(rct, Qt::AlignCenter, labelText);
}

void vuMeter::paintBar()
{
  QLinearGradient linGrad;
  int i;
  double bar;
  QRectF bar1;
  QRectF bar2;
  double length;
  QPainter painter(this);
  painter.setWindow(0, 0, w, h);
  painter.setRenderHint(QPainter::Antialiasing);
  if (horizontal)
    {
      linGrad.setStart(w,h); linGrad.setFinalStop(0,h);
      bar2=QRectF(OFFSET,3*BW,w-OFFSET-3*BW,h-6*BW);
      length = bar2.width();
      bar = abs(length * (1-(val-min)/(max-min)));
      bar1=QRectF(bar2.x()+bar2.width()-bar,bar2.y(),bar, bar2.height());
    }
  else
    {
      linGrad.setStart(w,0); linGrad.setFinalStop(w,h);
      bar2=QRectF(3*BW,4*BW,w-6*BW,h-1*OFFSET);
      length = bar2.height();
      bar = abs(length * (val-min)/(max-min));
      bar1=QRectF(bar2.x(),bar2.y(),bar2.width(), bar2.height()-bar);

    }

  linGrad.setColorAt(0, colHigh);
  linGrad.setColorAt(0.5, colMid);
  linGrad.setColorAt(1, colLow);
  linGrad.setSpread(QGradient::PadSpread);
  painter.setBrush(linGrad);
  painter.drawRect(bar2);


  // draw background bar
  painter.setBrush(QColor(40, 40, 40));

  painter.drawRect(bar1);
  painter.setPen(QPen(Qt::black, 2));
  for (i = 0; i <=divisions; i++)
    {
      if(horizontal)
        {
          painter.drawLine(bar2.left()+bar2.width()*i/divisions, bar2.top(), bar2.left()+bar2.width()*i/divisions, bar2.bottom());
        }
      else
        {
          painter.drawLine(bar2.left(), bar2.top()+bar2.height()*i/divisions, bar2.right(), bar2.top()+bar2.height()*i/divisions);
        }
    }


}


void vuMeter::setColorBg(QColor color)
{
  colBack = color;
  update();
}

void vuMeter::setColorValue(QColor color)
{
  colValue = color;
  update();
}

void vuMeter::setColorHigh(QColor color)
{
  colHigh = color;
  update();
}

void vuMeter::setColorMid(QColor color)
{
  colMid = color;
  update();
}

void vuMeter::setColors(QColor cL,QColor cM,QColor cH)
{
  colLow = cL;
  colMid = cM;
  colHigh = cH;
  update();
}

void vuMeter::setColorLow(QColor color)
{
  colLow = color;
  update();
}



void vuMeter::setValue(double value)
{
  if((fabs(1-(value/prevValue))<0.05)  && (slowCPU))
    {
      return;
    }
  if (value > max)
    {
      val = max;
    }
  else if (value < min)
    {
      val = min;
    }
  else
    {
      val = value;
    }
  prevValue=value;
  update();
}


void vuMeter::setMinimum(double minValue)
{
  if (minValue > max)
    {
      min = max;
      max = minValue;
      update();
    }
  else
    {
      min = minValue;
      update();
    }
}

void vuMeter::setMaximum(double maxValue)
{
  if (maxValue < min)
    {
      max = min;
      min = maxValue;
      update();
    }
  else
    {
      max = maxValue;
      update();
    }
}

QSize vuMeter::minimumSizeHint() const
{
  return QSize(10, 54);
}

QSize vuMeter::sizeHint() const
{
  return QSize(100, 540);
}


