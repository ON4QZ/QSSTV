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
#include "scopeplot.h"
#include "appglobal.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_legend_label.h"
#include "qwt_symbol.h"
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>

scopePlot::scopePlot(QString title, QWidget *parent) : QMainWindow(parent)
{
  wd=new QWidget(this);
  ui.setupUi(wd);
  mrk1=0;
  mrk2=0;
  initActions();
  initMenuBar();
  initToolBar();
  initStatusBar();
  curve1 = new QwtPlotCurve("Curve 1");
  curve2 = new QwtPlotCurve("Curve 2");
  curve3 = new QwtPlotCurve("Curve 3");
  curve4 = new QwtPlotCurve("Curve 4");

  xScaleMul=1.;
  xPrimeScaleMul=1.;
  xAltScaleMul=1;
  xAxisTitle="Samples";
  xAltAxisTitle="Time (s)";
  toggleMarker=false;

  showCrv1=true;
  showCrv2=true;
  showCrv3=true;
  showCrv4=true;
  setWindowTitle("Scope " +APPNAME);
  init(title);
}

scopePlot::~scopePlot()
{
  delete curve1;
  delete curve2;
  delete curve3;
  delete curve4;
  delete toolsMenu;
}

void scopePlot::setXScaleMultiplier(double mul)
{
  xPrimeScaleMul=mul;
  xScaleMul=mul;

}

void scopePlot::setAlternativeScaleMultiplier(double mul)
{
  xAltScaleMul=mul;
}

void scopePlot::initActions()
{


  zoomAction = new QAction(QIcon(":/icons/viewmagplus.png"), tr("&Zoom"), this);
  zoomAction->setCheckable(true);
  zoomAction->setStatusTip(tr("Zoom in or out"));
  connect(zoomAction, SIGNAL(toggled(bool)), this, SLOT(slotZoom(bool)));

}


void scopePlot::initMenuBar()
{
  toolsMenu=new QMenu(tr("&Zoom"));
  toolsMenu->addAction(zoomAction);
  menuBar()->addMenu(toolsMenu);
}

void scopePlot::initToolBar()
{
}

void scopePlot::initStatusBar()
{
}


void scopePlot::init(QString title)
{
  
  setup=true;
  setCentralWidget(wd);
  connect(ui.offsetWheel, SIGNAL(valueChanged(double)),SLOT(slotOffsetChanged(double )));
  connect(ui.rangeWheel, SIGNAL(valueChanged(double)), SLOT(slotRangeChanged(double )));
  connect(ui.samplesPushButton, SIGNAL(clicked()), this, SLOT(slotSamplesButtton()));
  plW=ui.plotWindow;
  plW->setTitle(title);
  plW->setCanvasBackground(Qt::darkBlue);
  curve1->attach(plW);
  curve2->attach(plW);
  curve3->attach(plW);
  curve4->attach(plW);
  plW->setAxisTitle(QwtPlot::xBottom,xAxisTitle);
  plW->setAxisScale(QwtPlot::xBottom, 0, 100);
  plW->setAxisTitle(QwtPlot::yLeft, "Values");
  plW->setAxisScale(QwtPlot::yLeft, -1.5, 1.5);

  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->setMajorPen(QPen(Qt::white, 0, Qt::DotLine));
  grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
  grid->attach(plW);
  QwtText m1("M1");
  m1.setColor(QColor(Qt::white));
  marker1=new QwtPlotMarker();
  marker1->setValue(0.0, 0.0);
  marker1->setLabel(m1);
  marker1->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
  marker1->setLinePen(QPen(QColor(200,150,0), 0, Qt::DashDotLine));
  marker1->setSymbol( new QwtSymbol(QwtSymbol::Diamond,QColor(Qt::green), QColor(Qt::green), QSize(7,7)));
  //	marker1->hide();
  marker1->attach(plW);



  QwtText m2("M2");
  m2.setColor(QColor(Qt::white));
  marker2=new QwtPlotMarker();
  marker2->setValue(0.0, 0.0);
  marker2->setLabel(m2);
  marker2->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
  marker2->setLinePen(QPen(QColor(200,150,0), 0, Qt::DashDotLine));
  marker2->setSymbol( new QwtSymbol(QwtSymbol::Diamond,QColor(Qt::yellow), QColor(Qt::yellow), QSize(7,7)));
  //	marker2->hide();
  marker2->attach(plW);



  legend = new QwtLegend;
  legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
  legend->setDefaultItemMode(QwtLegendData::Checkable);
  QPalette pal(legend->palette());
  pal.setColor(QPalette::Window,Qt::darkBlue);
  pal.setColor(QPalette::WindowText,Qt::white);
  pal.setColor(QPalette::Text,Qt::black);
  legend->setPalette(pal);
  plW->insertLegend(legend, QwtPlot::BottomLegend);

  picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, plW->canvas());
  picker->setStateMachine(new QwtPickerDragPointMachine());
  picker->setRubberBandPen(QColor(Qt::green));
  picker->setRubberBand(QwtPicker::CrossRubberBand);
  picker->setTrackerPen(QColor(Qt::white));
  picker->setEnabled(true);
  plW->replot();

  QwtPlotItemList items = plW->itemList( QwtPlotItem::Rtti_PlotCurve);
  for ( int i = 0; i < items.size(); i++ )
    {
      const QVariant itemInfo = plW->itemToInfo( items[i] );

      QwtLegendLabel *legendLabel =qobject_cast<QwtLegendLabel *>( legend->legendWidget( itemInfo ) );
      if (legendLabel )
        {
          legendLabel->setChecked( true );
        }

      items[i]->setVisible( true );
    }
  connect(picker, SIGNAL(moved(const QPointF  &)),SLOT(pickerMoved(const QPointF &)));
  connect(picker, SIGNAL(selected(const QPointF  &)), SLOT(pickerSelected(const QPointF  &)));
  connect(legend, SIGNAL(checked(const QVariant &, bool ,int)),SLOT(legendClicked(const QVariant &,bool)));
  connect(ui.nextButton, SIGNAL(clicked()),SLOT(slotNext()));
  connect(ui.previousButton, SIGNAL(clicked()),SLOT(slotPrevious()));
  plW->setAxisTitle(QwtPlot::xBottom,xAxisTitle);
  xOffset=0;
}


void scopePlot::setCurveOn(int i,bool b)
{
  QwtPlotItemList items = plW->itemList( QwtPlotItem::Rtti_PlotCurve);
  if(i>=items.size()) return;
  const QVariant itemInfo = plW->itemToInfo( items[i] );
  QwtLegendLabel *legendLabel =qobject_cast<QwtLegendLabel *>( legend->legendWidget( itemInfo ) );
  if (legendLabel )
    {
      legendLabel->setChecked(b);
    }

  items[i]->setVisible( b );

}

void scopePlot::slotSamplesButtton()
{
  long i;
  if (ui.samplesPushButton->isChecked())
    {
      ui.samplesPushButton->setText(xAltAxisTitle);
      plW->setAxisTitle(QwtPlot::xBottom,xAltAxisTitle);
      xScaleMul=xAltScaleMul;
      ui.offsetWheel->setSingleStep(10*xScaleMul);
      ui.rangeWheel->setSingleStep(10*xScaleMul);
    }
  else
    {
      ui.samplesPushButton->setText(xAxisTitle);
      plW->setAxisTitle(QwtPlot::xBottom,xAxisTitle);
      xScaleMul=xPrimeScaleMul;
      ui.offsetWheel->setSingleStep(1);

      ui.rangeWheel->setSingleStep(1);

    }
  for(i=0;i<x.size();i++)
    {
      x[i]=(double)(i+xOffset)*xScaleMul;
//      x[i]=(double)i*xScaleMul;
    }

  ui.offsetWheel->setPageStepCount(10);
  ui.rangeWheel->setPageStepCount(10);
  if(x.size()==c1.size()) curve1->setSamples(x.data(), c1.data(), x.size());
  if(x.size()==c3.size()) curve2->setSamples(x.data(), c3.data(), x.size());
  if(x.size()==c3.size()) curve3->setSamples(x.data(), c3.data(), x.size());
  if(x.size()==c4.size()) curve4->setSamples(x.data(), c4.data(), x.size());
  setupWheels(x.size());
  plW->replot();
}







void scopePlot::plot1DUpdate(double *data)
{
  for (long i = 0; i < c1.size(); i++)
    {
      c1[i]=data[i];
    }
  curve1->setSamples(x.data(), c1.data(), x.size());
  plW->replot();
}

void scopePlot::plotData(unsigned int size,
                         short int * iData, QString curve1Name,QString yLLabel,
                         double * dData, QString curve2Name, QString yRLabel)
{	
  add1(iData,size,curve1Name,yLLabel);
  add3(dData,size,curve2Name,yRLabel);
  show();
}

void  scopePlot::plotData(unsigned int size,
                          double * dData1, QString curve1Name, QString yLLabel,
                          double * dData2, QString curve2Name, QString yRLabel)
{	
  add1(dData1,size,curve1Name,yLLabel);
  add3(dData2,size,curve2Name,yRLabel);
  show();
}



void scopePlot::add1(short int *data, unsigned long len,QString name,QString yLeftLabel)
{
  x.resize(len);
  c1.resize(len);
  for (unsigned long i = 0; i < len; i++)
    {

      x[i]=(double)(i+xOffset)*xScaleMul;
      c1[i]=(double)data[i];
    }
  plot1(name,yLeftLabel);
}

void scopePlot::add1(double *data, unsigned long len,QString curveName,QString yLeftLabel)
{
  x.resize(len);
  c1.resize(len);
  for (unsigned long i = 0; i < len; i++)
    {
      x[i]=(double)(i+xOffset)*xScaleMul;
      c1[i]=data[i];
    }
  plot1(curveName,yLeftLabel);
}


void scopePlot::add2(double *data, unsigned long len,QString curveName)
{
  c2.resize(len);
  for (unsigned long i = 0; i < len; i++)
    {
      c2[i]=data[i];
    }
  plot2(curveName);
}

void scopePlot::add3(double *data, unsigned long len,QString curveName,QString yRightLabel)
{
  c3.resize(len);
  for (unsigned long i = 0; i < len; i++)
    {
      c3[i]=data[i];
    }
  plot3(curveName,yRightLabel);
}

void scopePlot::add4(double *data, unsigned long len,QString curveName)
{
  c4.resize(len);
  for (unsigned long i = 0; i < len; i++)
    {
      c4[i]=data[i];
    }
  plot4(curveName);
}

void scopePlot::show()
{
  QMainWindow::show();
  plW->show();
  plW->replot();
}

void scopePlot::refresh()
{

  plW->replot();
}



void scopePlot::plot1(QString curveName,QString yLeftLabel)
{
  plW->setAxisTitle(QwtPlot::yLeft, yLeftLabel);
  plW->setAxisAutoScale(QwtPlot::yLeft);
  plW->setAxisAutoScale(QwtPlot::xBottom);
  curve1->setTitle(curveName);
  curve1->setPen(QPen(Qt::yellow));
  curve1->setYAxis(QwtPlot::yLeft);
  curve1->setSamples(x.data(), c1.data(), x.size());
  setupWheels(x.size());
}



void scopePlot::plot2(QString curveName)
{

  // axes
  plW->setAxisAutoScale(QwtPlot::yLeft);
  plW->enableAxis(QwtPlot::yLeft);
  curve2->setTitle(curveName);
  curve2->setPen(QPen(Qt::red));
  curve2->setYAxis(QwtPlot::yLeft);
  curve2->setSamples(x.data(), c2.data(), x.size());
  plW->replot();
}

void scopePlot::plot3(QString curveName,QString yRightLabel)
{

  // axes
  plW->setAxisTitle(QwtPlot::yRight,yRightLabel);
  plW->enableAxis(QwtPlot::yRight);
  curve3->setTitle(curveName);
  curve3->setPen(QPen(Qt::green));
  curve3->setYAxis(QwtPlot::yRight);
  curve3->setSamples(x.data(), c3.data(), x.size());
  plW->setAxisAutoScale(QwtPlot::yRight);
  plW->replot();
}

void scopePlot::plot4(QString curveName)
{

  // axes
  plW->setAxisAutoScale(QwtPlot::yRight);
  plW->enableAxis(QwtPlot::yRight);
  curve4->setTitle(curveName);
  curve4->setPen(QPen(Qt::white));
  curve4->setYAxis(QwtPlot::yRight);
  curve4->setSamples(x.data(), c4.data(), x.size());
  plW->replot();
}




void scopePlot::slotZoom(bool)
{
}

void scopePlot::slotOffsetChanged(double ioffset)
{
  if (setup) return;
  if((ioffset-range/2)<startPoint)
    {
      ui.offsetWheel->setValue(startPoint+range/2);
      return;
    }
  if((ioffset+range/2)>endPoint)
    {
      ui.offsetWheel->setValue(endPoint-range/2);
      return;
    }
  dispCenter=ioffset;
  plW->setAxisScale(QwtPlot::xBottom,dispCenter-range/2,dispCenter+range/2);
  plW->replot();
}

void scopePlot::slotRangeChanged(double irange)
{
  if (setup) return;

  if((dispCenter-irange/2)<startPoint)
    {
      ui.rangeWheel->setValue((dispCenter-startPoint)*1.999);
      return;
    }
  if((dispCenter+irange/2)>endPoint)
    {
      ui.rangeWheel->setValue((endPoint-dispCenter)*1.999);
      return;
    }

  range=irange;
  plW->setAxisScale(QwtPlot::xBottom,dispCenter-range/2,dispCenter+range/2);
  plW->replot();
}

void scopePlot::setupWheels(int size)
{
  setup=true;
  // offset is from 0 to size-range
  // range is from 10 to
  if (x.size()==0)
    {
      QMessageBox::warning(0,"Scope Plot",
                           "No data in Scope Plot" ,
                           QMessageBox::Ok);
      return;
    }
  blockSignals(true);
  startPoint=x[0];
  endPoint=x[size-1];
  range=endPoint-startPoint;
  dispCenter=(endPoint+startPoint)/2;
  ui.offsetWheel->setMass(0.5);
  ui.rangeWheel->setMass(0.5);

  ui.offsetWheel->setRange(startPoint, endPoint);
  ui.offsetWheel->setTotalAngle(3600.0);
  ui.rangeWheel->setRange(range/200., range);
  //	range=endPoint-startPoint;
  ui.rangeWheel->setTotalAngle(3600.0);
  blockSignals(false);
  ui.offsetWheel->setValue(dispCenter);
  ui.rangeWheel->setValue(range);
  setup=false;
  plW->setAxisScale(QwtPlot::xBottom,dispCenter-range/2,dispCenter+range/2);
}


void scopePlot::pickerMoved(const QPointF &pos)
{
  QString info;
  info=QString("x=%1,yL=%2, yR=%3")

               .arg(plW->invTransform(QwtPlot::xBottom, pos.x()),7,'g')
               .arg(plW->invTransform(QwtPlot::yLeft  , pos.y()),7,'g')
               .arg(plW->invTransform(QwtPlot::yRight,  pos.y()),7,'g');

  // ui.positionLabel->setText(info);
}


void scopePlot::pickerSelected(const QPointF  &pos)
{
  if (!toggleMarker)
    {
      marker1->setValue(pos);
    }
  else
    {
      marker2->setValue(pos);
    }
  toggleMarker=!toggleMarker;
  showMarker();
  plW->replot();
}


void scopePlot::showMarker()
{
  QString t1,t2,t3;

  t1="M1 : "+QString::number(marker1->xValue(),'g',7).rightJustified(11)+QString::number(marker1->yValue(),'g',7).rightJustified(11);
  t2="M2 : "+QString::number(marker2->xValue(),'g',7).rightJustified(11)+QString::number(marker2->yValue(),'g',7).rightJustified(11);
  t3="DIF: "+QString::number(marker2->xValue()-marker1->xValue(),'g',7).rightJustified(11)+QString::number(marker2->yValue()-marker1->yValue(),'g',7).rightJustified(11);
  ui.marker1Label->setText(t1);
  ui.marker2Label->setText(t2);
  ui.marker3Label->setText(t3);
}



void scopePlot::legendClicked(const QVariant &itemInfo, bool on)
{
  QwtPlotItem *plotItem = plW->infoToItem( itemInfo );
  if ( plotItem )
    {
      plotItem->setVisible( on );
      plW->replot();
    }
}


