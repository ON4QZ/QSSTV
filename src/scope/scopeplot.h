/***************************************************************************
 *   Copyright (C) 2008 by Johan Maes                                      *
 *   on4qz@telenet.be                                                      *
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

#ifndef SCOPEPLOT_H
#define SCOPEPLOT_H
#include <QtGui>
#include <QMainWindow>
#include "ui_plotform.h"
#include "qwt_plot.h"
#include "qwt_legend.h"

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotPicker;



/**Data  plotting in scope format
  *@author Johan Maes -- ON4QZ 
  */

class scopePlot : public QMainWindow  {
   Q_OBJECT
public: 
	scopePlot(QString title,QWidget *parent=0);
	~scopePlot();
  void setXScaleMultiplier(double mul);
  void setAlternativeScaleMultiplier(double mul);
  void setOffset(unsigned int offset) {xOffset=offset;}
  void init(QString title=QString());

  void add1(short int *data, unsigned long len,QString curveName,QString yLeftLabel);
  void add1(double *data, unsigned long len,QString curveName,QString yLeftLabel);
  void add2(double *data, unsigned long len,QString curveName);
  void add3(double *data, unsigned long len,QString curveName,QString yRightLabel);
  void add4(double *data, unsigned long len,QString curveName);
  void plot1(QString name,QString yLeftLabel);
  void plot2(QString curveName);
  void plot3(QString curveName,QString yRightLabel);
  void plot4(QString curveName);
	void plotData(unsigned int size,
                      short int * iData, QString curve1Name, QString yLLabel,
											double * dData, QString curve2Name, QString yRLabel);
	void plotData(unsigned int size,
                      double * dData1, QString curve1Name, QString yLLabel,
											double * dData2, QString curve2Name, QString yRLabel);
											
											
	void show();
	void plot1DUpdate(double *data);
  void XYL(unsigned int i,int ix,int iy)
    {
      x[i]=(double)ix;
      c1[i]=(double)iy;
    }
  void X(unsigned int i,int ix)
    {
      x[i]=(double)ix;
      
    }
  void YL(unsigned int i,int iy)
    {
      c1[i]=(double)iy;
    }
  void YR(unsigned int i,int iy)
    {
      c3[i]=(double)iy;

    }
  void XYLYR(unsigned int i,int ix,int iyl,int iyr)
    {
      x[i]=(double)ix;
      c1[i]=(double)iyl;
      c3[i]=(double)iyr;
    }

  void XYL(unsigned int i,double ix,double iy)
    {
      x[i]=ix;
      c1[i]=iy;
    }
    
  void X(unsigned int i,double ix)
    {
      x[i]=ix;

    }
  void YL(unsigned int i,double iy)
    {
      c1[i]=iy;
    }
  void YR(unsigned int i,double iy)
    {
      c3[i]=iy;

    }
  void XYLYR(unsigned int i,double ix,double iyl,double iyr)
    {
      x[i]=ix;
      c1[i]=iyl;
      c3[i]=iyr;
    }
  void resize(unsigned long i)
    {
      x.resize(i);
      c1.resize(i);
      c3.resize(i);
    }
	void refresh();
public slots:
	void slotZoom(bool b);
	void slotOffsetChanged(double offset);
	void slotRangeChanged(double range);
  void pickerMoved(const QPointF &pos);
  void pickerSelected(const QPointF &pos);
//  void plotMouseMoved(const QMouseEvent &e);
//	void plotMouseReleased(const QMouseEvent &e);
  void legendClicked(const QVariant &itemInfo, bool on);
	void slotNext()
		{
			emit next();
		}
	void slotPrevious()
		{
			emit previous();
		}
	void slotSamplesButtton();
  void setCurveOn(int i,bool b);
signals:
	void next();
	void previous();  
private:
	QwtPlot *plW;
	QToolBar *toolsToolbar;
	QMenu *toolsMenu;
	QAction *zoomAction;
//	plotForm *pl;

	QVector<double> x;
  QVector<double> c1;
  QVector<double> c2;
	QVector<double> c3;
  QVector<double> c4;
	double startPoint;
	double endPoint;
	double dispCenter;

	
	void initActions();
	void initMenuBar();
	void initToolBar();
	void initStatusBar();
	void setupWheels(int size);
	double xScaleMul;
	double xPrimeScaleMul;
	double xAltScaleMul;
	bool setup;
	long int mrk1,mrk2;
	void showMarker();
	bool toggleMarker;
	bool showCrv1;
	bool showCrv2;
	bool showCrv3;
  bool showCrv4;
	Ui::plotForm ui;
	QWidget *wd;

	QwtPlotCurve *curve1; 
	QwtPlotCurve *curve2;
	QwtPlotCurve *curve3;
  QwtPlotCurve *curve4;
	QwtPlotMarker *marker1;
	QwtPlotMarker *marker2;
	QwtLegend *legend;
	QwtPlotPicker *picker;
  QString xAxisTitle;
  QString xAltAxisTitle;
	double range;
  unsigned int xOffset;

};

#endif
