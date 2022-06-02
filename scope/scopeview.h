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
#ifndef SCOPEVIEW_H
#define SCOPEVIEW_H
#include "appglobal.h"
#include <QString>

#include "scopeplot.h"

#define SCOPEMAXDATA	300000
/**
	@author Johan Maes <on4qz@telenet.be>
*/

enum ecurve{SCDATA1,SCDATA2,SCDATA3,SCDATA4};
#define NUMCURVES 4

class scopeView: public scopePlot
{
public:
  scopeView(QString title);
  ~scopeView();
  void init();
  void clear();
  void setOffset (int xoffset);

  void addData(ecurve idx,double  *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,float   *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,qint8   *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,quint8  *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,qint16  *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,quint16 *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,qint32  *data,unsigned int position,unsigned int len);
  void addData(ecurve idx,quint32 *data,unsigned int position,unsigned int len);


  void show(bool data,bool sync,bool state,bool d4);
  void setCurveName(QString title,int idx);
  void setAxisTitles(QString x,QString yData1,QString yData2)
  {
    xTitle=x;
    yLeftTitle=yData1;
    yRightTitle=yData2;
  }

private:
  double array1[SCOPEMAXDATA];
  double array2[SCOPEMAXDATA];
  double array3[SCOPEMAXDATA];
  double array4[SCOPEMAXDATA];
  unsigned int index;
  QString curveNameArray[NUMCURVES];
  QString xTitle;
  QString yLeftTitle;
  QString yRightTitle;
  unsigned int xOffset;
};

#endif

