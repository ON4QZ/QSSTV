#include "gallerywidget.h"
#include "ui_gallerywidget.h"
#include "configparams.h"
#include "logging.h"
#include "appglobal.h"
#include <QSplashScreen>
#include "dispatch/dispatcher.h"
#include <QFileInfo>
#include <QStatusBar>
#include "txwidget.h"



galleryWidget::galleryWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::galleryWidget)
{
  ui->setupUi(this);
  ui->tabWidget->setCurrentIndex(0);
  ui->templateMatrix->setSortFlag(QDir::Name);

}

galleryWidget::~galleryWidget()
{
  writeSettings();
  delete ui;

}




void galleryWidget::init()
{
  readSettings();
}


void galleryWidget::changedMatrix()
{
  changedMatrix(imageViewer::RXSSTVTHUMB);
  changedMatrix(imageViewer::RXDRMTHUMB);
  changedMatrix(imageViewer::TXSSTVTHUMB);
  changedMatrix(imageViewer::TXDRMTHUMB);
  changedMatrix(imageViewer::TXSTOCKTHUMB);
  changedMatrix(imageViewer::TEMPLATETHUMB);
}

void galleryWidget::slotDirChanged(QString dn)
{
  if(dn==txStockImagesPath)
    {
      ui->txStockMatrix->changed();
    }
  if(dn==templatesPath)
    {
      ui->templateMatrix->changed();
      txWidgetPtr->setupTemplatesComboBox();
    }
}


void galleryWidget::changedMatrix(imageViewer::thumbType itype)
{
  switch(itype)
    {
    case imageViewer::RXSSTVTHUMB:
      ui->rxSSTVMatrix->init(galleryRows,galleryColumns,rxSSTVImagesPath,imageViewer::RXSSTVTHUMB);
      break;
    case imageViewer::RXDRMTHUMB:
      ui->rxDRMMatrix->init(galleryRows,galleryColumns,rxDRMImagesPath,imageViewer::RXDRMTHUMB);
      break;
    case imageViewer::TXSSTVTHUMB:
      ui->txSSTVMatrix->init(galleryRows,galleryColumns,txSSTVImagesPath,imageViewer::TXSSTVTHUMB);
      break;
    case imageViewer::TXDRMTHUMB:
      ui->txDRMMatrix->init(galleryRows,galleryColumns,txDRMImagesPath,imageViewer::TXDRMTHUMB);
      break;
    case imageViewer::TXSTOCKTHUMB:
      ui->txStockMatrix->init(galleryRows,galleryColumns,txStockImagesPath,imageViewer::TXSTOCKTHUMB);
      break;
    case imageViewer::TEMPLATETHUMB:
      ui->templateMatrix->init(galleryRows,galleryColumns,templatesPath,imageViewer::TEMPLATETHUMB);
      break;
    default:
      break;
    }
}



/*!
  reads the settings (saved images for tx,rx,templates)
*/

void galleryWidget::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("Gallery");

  splashStr+=QString( "Loading RX images" ).rightJustified(25,' ')+"\n";
  splashPtr->showMessage ( splashStr ,Qt::AlignLeft,Qt::white);
  qApp->processEvents();
  ui->rxSSTVMatrix->init(galleryRows,galleryColumns,rxSSTVImagesPath,imageViewer::RXSSTVTHUMB);
  ui->rxDRMMatrix->init(galleryRows,galleryColumns,rxDRMImagesPath,imageViewer::RXDRMTHUMB);

  splashStr+=QString( "Loading TX images" ).rightJustified(25,' ')+"\n";
  splashPtr->showMessage ( splashStr ,Qt::AlignLeft,Qt::white);
  qApp->processEvents();
  ui->txSSTVMatrix->init(galleryRows,galleryColumns,txSSTVImagesPath,imageViewer::TXSSTVTHUMB);
  ui->txDRMMatrix->init(galleryRows,galleryColumns,txDRMImagesPath,imageViewer::TXDRMTHUMB);
  ui->txStockMatrix->init(galleryRows,galleryColumns,txStockImagesPath,imageViewer::TXSTOCKTHUMB);

  splashStr+=QString( "Loading Templates" ).rightJustified(25,' ')+"\n";
  splashPtr->showMessage ( splashStr ,Qt::AlignLeft,Qt::white);
  ui->templateMatrix->init(galleryRows,galleryColumns,templatesPath,imageViewer::TEMPLATETHUMB);
}

/*!
  writes the settings (saved images for tx,rx,templates)
*/
void galleryWidget::writeSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ( "Gallery" );
  qSettings.endGroup();
}

/*!
  setup of the user interface
*/



/*!
  closeEvent signals the dispatcher to initiate  program exit.
*/


/*!
  get the filename of a template

  \param[in] tm index of template
  \return QString containing filename, check with QString.isNull for validity
*/

QString galleryWidget::getTemplateFileName ( int tm )
{
  int i;
  QString str;
  QStringList slTemp;
  if(tm>=0)
    {
      for(i=0;i<ui->templateMatrix->getFileList().count();i++)
        {
          str=ui->templateMatrix->getFileList().at(i).absoluteFilePath();
          slTemp.append(str);
        }
      slTemp.sort();
      return slTemp.at(tm);
    }
  else return QString();
}

const QStringList &galleryWidget::getFilenames()
{
  QString str;
  sl.clear();
  int i;
  for(i=0;i<ui->templateMatrix->getFileList().count();i++)
    {
      str=getTemplateFileName (i);
      QFileInfo fi(str);
      sl.append(fi.baseName());
    }
  return sl;
}

/*!
  loads an image in the rximages gallery in a new position

  \param[in] fn the filename

*/

void galleryWidget::putRxImage ( QString fn )
{
  if(transmissionModeIndex==TRXSSTV)
    {
      statusBarPtr->showMessage ( "Saved: "+fn );
      ui->rxSSTVMatrix->changed();
    }
  else
    {
      statusBarPtr->showMessage ( "Saved: "+fn+QString("  (%1dB)").arg(lastAvgSNR,0,'f',0) );
      ui->rxDRMMatrix->changed();
    }
}

void galleryWidget::txImageChanged()
{
  if(transmissionModeIndex==TRXSSTV)
    {
      ui->txSSTVMatrix->changed();
    }
  else
    {
      ui->txDRMMatrix->changed();
    }
}

void galleryWidget::txStockImageChanged()
{
  ui->txStockMatrix->changed();
}


/*!
  loads an image in the rximages gallery in a new position

  \return QString containing filename, check with QString.isEmpty for validity

*/
QString galleryWidget::getLastRxImage()
{
  return ui->rxDRMMatrix->getLastFile();
}






