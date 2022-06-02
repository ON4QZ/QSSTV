#include "spectrumwidget.h"
#include "appglobal.h"
#include "ui_spectrumwidget.h"
#include "supportfunctions.h"
#include "logging.h"
#include "markerwidget.h"

spectrumWidget::spectrumWidget(QWidget *parent) : QFrame(parent),  ui(new Ui::spectrumWidget)
{
  ui->setupUi(this);
  readSettings();
  connect(ui->maxDbSpinbox,SIGNAL(valueChanged(int)),SLOT(slotMaxDbChanged(int)));
  connect(ui->rangeSpinbox,SIGNAL(valueChanged(int)),SLOT(slotRangeChanged(int)));
  connect(ui->avgDoubleSpinBox,SIGNAL(valueChanged(double)),SLOT(slotAvgChanged(double)));
}

spectrumWidget::~spectrumWidget()
{
  writeSettings();
  delete ui;
}

void spectrumWidget::init(int length,int numBlocks,int isamplingrate)
{
//  addToLog(QString("Size: %1, Number of blocks %2, Samplingrate %3").arg(length).arg(numBlocks).arg(isamplingrate),LOGFFT);
  fftFunc.init(length,numBlocks,isamplingrate);
  ui->spectrWidget->init(length,numBlocks,isamplingrate);
  ui->waterfallWidget->init(length,numBlocks,isamplingrate);
}


void spectrumWidget::realFFT(double *iBuffer)
{
    fftFunc.realFFT(iBuffer);
     ui->spectrWidget->showFFT(fftFunc.out);
    ui->waterfallWidget->showFFT(fftFunc.out);
}


void spectrumWidget::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("SPECTRUM");
  maxdb=qSettings.value("maxdb",-25).toInt();
  range=qSettings.value("range",35).toInt();
  avg=qSettings.value("avg",0.90).toDouble();
  qSettings.endGroup();
  setParams();
}


void spectrumWidget::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("SPECTRUM");
  qSettings.setValue( "maxdb",maxdb);
  qSettings.setValue( "range",range);
  qSettings.setValue("avg",avg);
  qSettings.endGroup();
}

void spectrumWidget::getParams()
{
  getValue(maxdb,ui->maxDbSpinbox);
  getValue(range,ui->rangeSpinbox);
  getValue(avg,ui->avgDoubleSpinBox);
}

void spectrumWidget::setParams()
{
  setValue(maxdb,ui->maxDbSpinbox);
  setValue(range,ui->rangeSpinbox);
  setValue(avg,ui->avgDoubleSpinBox);
  slotMaxDbChanged(maxdb);
  slotRangeChanged(range);
  slotAvgChanged(avg);

}

void spectrumWidget::displaySettings(bool drm)
{
  ui->spectrWidget->displayWaterfall(false);
  ui->waterfallWidget->displayWaterfall(true);
  if(drm)
  {
      ui->spectrWidget->setMarkers(725,1475,1850);
      ui->waterfallWidget->setMarkers(725,1475,1850);
      ui->markerLabelSpectrum->setMarkers(725,1475,1850);
      ui->markerLabelWF->setMarkers(725,1475,1850);

    }
  else
    {
      ui->spectrWidget->setMarkers(1200,1500,2300);
      ui->waterfallWidget->setMarkers(1200,1500,2300);
      ui->markerLabelSpectrum->setMarkers(1200,1500,2300);
      ui->markerLabelWF->setMarkers(1200,1500,2300);
    }
}

QImage *spectrumWidget::getImage()
{
  return ui->waterfallWidget->getImage();
}

void spectrumWidget::slotMaxDbChanged(int mb)
{
  ui->spectrWidget->setMaxDb(mb);
  ui->waterfallWidget->setMaxDb(mb);
  maxdb=mb;
}
void spectrumWidget::slotRangeChanged(int rg)
{
  ui->spectrWidget->setRange(rg);
  ui->waterfallWidget->setRange(rg);
  range=rg;
}

void spectrumWidget::slotAvgChanged(double d)
{
   ui->spectrWidget->setAvg(d);
   ui->waterfallWidget->setAvg(d);
}


