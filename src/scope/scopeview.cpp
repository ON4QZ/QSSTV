/***************************************************************************
 *   Copyright (C) 2005 by Johan Maes   *
 *   on4qz@telenet.be   *
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
 *   In addition, as a special exception, the copyright holders give       *
 *   permission to link the code of this program with any edition of       *
 *   the Qt library by Trolltech AS, Norway (or with modified versions     *
 *   of Qt that use the same license as Qt), and distribute linked         *
 *   combinations including the two.  You must obey the GNU General        *
 *   Public License in all respects for all of the code used other than    *
 *   Qt.  If you modify this file, you may extend this exception to        *
 *   your version of the file, but you are not obligated to do so.  If     *
 *   you do not wish to do so, delete this exception statement from        *
 *   your version.                                                         *
 ***************************************************************************/
#include "scopeview.h"
#include "loggingparams.h"



scopeView::scopeView(QString title) : scopePlot(title)
{
  xOffset=0;
  init();
}


scopeView::~scopeView()
{
}

void scopeView::init()
{
  setCurveName("data1",SCDATA1);
  setCurveName("data2",SCDATA2);
  setCurveName("data3",SCDATA3);
  setCurveName("data4",SCDATA4);
  clear();
}

void scopeView::clear()
{
  int i;
  index=0;
  for (i=0;i<SCOPEMAXDATA;i++)
    {
      array1[i]=0.;
      array2[i]=0.;
      array3[i]=0.;
      array4[i]=0.;
    }
}

void scopeView::setOffset (int xoffset)
{
  xOffset=xoffset;
  scopePlot::setOffset(xOffset);
}

void scopeView::addData(ecurve idx,double *data,unsigned int position,unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1:
      ar=array1;
      break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {

      ar[i]=data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}

void scopeView::addData(ecurve idx,float *data,unsigned int position,unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1: ar=array1; break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {

      ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}


void scopeView::addData(ecurve idx, qint8 *data, unsigned int position, unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1: ar=array1; break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {
       ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}

void scopeView::addData(ecurve idx,quint8 *data, unsigned int position, unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1: ar=array1; break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {
       ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}


void scopeView::addData(ecurve idx,qint16 *data,unsigned int position,unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1: ar=array1; break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {

      ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}

void scopeView::addData(ecurve idx,quint16 *data,unsigned int position,unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1: ar=array1; break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {

      ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}


void scopeView::addData(ecurve idx,qint32 *data,unsigned int position,unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1: ar=array1; break;
    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {

      ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}

void scopeView::addData(ecurve idx,quint32 *data,unsigned int position,unsigned int len)
{
  unsigned int i,j;
  double *ar=NULL;
  if(position<xOffset) return;
  switch(idx)
    {
    case SCDATA1:
      ar=array1;
      break;

    case SCDATA2: ar=array2; break;
    case SCDATA3: ar=array3; break;
    case SCDATA4: ar=array4; break;
    }
  for(i=position-xOffset,j=0;i<(position+len-xOffset)&& (i<SCOPEMAXDATA);i++,j++)
    {

      ar[i]=(double)data[j];
    }
  if(i>SCOPEMAXDATA) i=SCOPEMAXDATA;
  index=i;
  addToLog(QString("data1 %1").arg(index+xOffset),LOGSCOPE);
}



void scopeView::setCurveName(QString title,int idx)
{
  if((idx>=0)&&(idx<=SCDATA4))
    {
      curveNameArray[idx]=title;
    }
}
void scopeView::show(bool d1,bool d2,bool d3,bool d4)
{
  if(d1)
    {
      add1(array1,index,curveNameArray[SCDATA1],yLeftTitle);
      setCurveOn(SCDATA1,true);
    }
  else setCurveOn(SCDATA1,false);
  if(d2)
    {
      add2(array2,index,curveNameArray[SCDATA2]);
      setCurveOn(SCDATA2,true);
    }
  else setCurveOn(SCDATA2,false);

  if (d3)
    {
      add3(array3,index,curveNameArray[SCDATA3],yRightTitle);
      setCurveOn(SCDATA3,true);
    }
  else setCurveOn(SCDATA3,false);
  if (d4)
    {
      add4(array4,index,curveNameArray[SCDATA4]);
      setCurveOn(SCDATA4,true);
    }
  else setCurveOn(SCDATA4,false);
  scopePlot::show();
}
