#include "rxwidget.h"
#include "appglobal.h"
#include "dispatcher.h"
#include "dirdialog.h"
#include "rxfunctions.h"
#include "imageviewer.h"
#include "sstvparam.h"
#include "filterparam.h"
#include "mainwindow.h"
#include "soundbase.h"
#include "vumeter.h"
#include "sstvrx.h"
#include "guiconfig.h"
#include "mainwindow.h"
#include "configparams.h"
#include "ftpthread.h"


rxWidget::rxWidget(QWidget *parent):QWidget(parent),ui(new Ui::rxWidget)
{
  int i;
  ui->setupUi(this);
  rxFunctionsPtr=new rxFunctions();
  ui->syncWidget->setHorizontal(false);
  ui->syncWidget->setLabelText("S");

  ui->vuWidget->setHorizontal(true);
  ui->vuWidget->setLabelText("V");
  imageViewerPtr=ui->imageFrame;


  ui->sstvModeComboBox->addItem("Auto");
  for(i=0;i<NUMSSTVMODES;i++)
    {
      ui->sstvModeComboBox->addItem(getSSTVModeNameLong((esstvMode)i));
    }
  foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
      QString text = tr("%1").arg(QString(format));
      ui->defaultImageFormatComboBox->addItem(text);
    }
  QStringList strLst;
  strLst << "Low" << "Normal"  << "High" << "DX";
  ui->sensitivityComboBox->insertItems(0,strLst);
  connect(&rxFunctionsPtr->sstvRxPtr->syncWideProc,SIGNAL(callReceived(QString)),SLOT(slotNewCall(QString)));
  connect(rxFunctionsPtr->sstvRxPtr,SIGNAL(resetCall()),SLOT(slotResetCall()));
  connect(ui->logPushButton,SIGNAL(clicked()),SLOT(slotLogCall()));
  connect(ui->whoPushButton,SIGNAL(clicked()),SLOT(slotWho()));
  connect(&ff,SIGNAL(listingDone(bool)),SLOT(slotWhoResult(bool)));
  doRemove=false;
}

rxWidget::~rxWidget()
{
  writeSettings();
  rxFunctionsPtr->terminate();
  delete rxFunctionsPtr;
  delete notifyRXIntfPtr;
}

void rxWidget::init()
{
  splashStr+=QString( "Setting up RX" ).rightJustified(25,' ')+"\n";
  splashPtr->showMessage ( splashStr ,Qt::AlignLeft,Qt::white);

  qApp->processEvents();
  readSettings();
  //  imageViewerPtr->createImage(QSize(320,256),QColor(0,0,128),imageStretch);
  imageViewerPtr->createImage(QSize(320,256),imageBackGroundColor,imageStretch);
  imageViewerPtr->setType(imageViewer::RXIMG);
  setSettingsTab();

  rxFunctionsPtr->init();


  // make connections after initialization
  connect(ui->startToolButton, SIGNAL(clicked()),SLOT(slotStart()));
  connect(ui->stopToolButton, SIGNAL(clicked()),SLOT(slotStop()));
  connect(ui->resyncToolButton,SIGNAL(clicked()),SLOT(slotResync()));
  connect(ui->autoSaveCheckBox,SIGNAL(clicked()),SLOT(slotGetParams()));
  connect(ui->autoSlantAdjustCheckBox,SIGNAL(clicked()),SLOT(slotGetParams()));




  connect(ui->sensitivityComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotGetParams()));
  connect(ui->settingsTableWidget,SIGNAL(currentChanged(int)),this, SLOT(slotTransmissionMode(int)));
  connect(ui->eraseToolButton, SIGNAL(clicked()),SLOT(slotErase()));
  connect(ui->saveToolButton, SIGNAL(clicked()),SLOT(slotSave()));

  if(slowCPU)
    {
      ui->drmFACLabel->hide();
      ui->drmMSCLabel->hide();
      ui->drmMSCWidget->hide();
      ui->drmFACWidget->hide();
    }
  if(slowCPU || lowRes)
    {
      ui->rxNotificationList->hide();
      ui->whoPushButton->hide();
      //      ui->whoSpacer->hide();
    }
  setOnlineStatus(true, onlineStatusText);
}

void rxWidget::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("RX");
  autoSlantAdjust=qSettings.value("autoSlantAdjust",false).toBool();
  autoSave=qSettings.value("autoSave",true).toBool();
  sensitivity=qSettings.value("sensitivity",1).toInt();
  sstvModeIndexRx=(esstvMode)qSettings.value("sstvModeIndexRx",0).toInt();
  defaultImageFormat=qSettings.value("defaultImageFormat","png").toString();
  minCompletion=qSettings.value("minCompletion",25).toInt();
  setParams();
  qSettings.endGroup();

}

void rxWidget::writeSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("RX");
  getParams();
  qSettings.setValue("autoSlantAdjust",autoSlantAdjust);
  qSettings.setValue("autoSave",autoSave);
  qSettings.setValue("sensitivity",sensitivity);
  qSettings.setValue("sstvModeIndexRx",sstvModeIndexRx);
  qSettings.setValue("defaultImageFormat",defaultImageFormat);
  qSettings.setValue("minCompletion",minCompletion);
  qSettings.endGroup();
}

void rxWidget::getParams()
{
  int temp;
  getValue(autoSlantAdjust,ui->autoSlantAdjustCheckBox);
  getValue(autoSave,ui->autoSaveCheckBox);
  getIndex(sensitivity,ui->sensitivityComboBox);
  getIndex(temp,ui->sstvModeComboBox);
  sstvModeIndexRx=(esstvMode)temp;
  getValue(defaultImageFormat,ui->defaultImageFormatComboBox);
  getValue(minCompletion,ui->completeSpinBox);
}

void rxWidget::setParams()
{
  setValue(autoSlantAdjust,ui->autoSlantAdjustCheckBox);
  setValue(autoSave,ui->autoSaveCheckBox);
  setIndex(sensitivity,ui->sensitivityComboBox);
  setIndex(sstvModeIndexRx,ui->sstvModeComboBox);
  setValue(defaultImageFormat,ui->defaultImageFormatComboBox);
  setValue(minCompletion,ui->completeSpinBox);
}

void rxWidget::slotGetParams()
{
  getParams();
}


void rxWidget::slotStart()
{
  getParams();
  dispatcherPtr->startRX();
}

void rxWidget::slotStop()
{
  getParams();
  dispatcherPtr->idleAll();
}

void rxWidget::slotResync()
{
  rxFunctionsPtr->restartRX();
}

void rxWidget::slotTransmissionMode(int rxtxMode)
{
  emit modeSwitch(rxtxMode);
}


void rxWidget::changeTransmissionMode(int rxtxMode)
{
  transmissionModeIndex=(etransmissionMode)rxtxMode;
  dispatcherPtr->idleAll();
  setSettingsTab();
  switch(transmissionModeIndex)
    {
    case TRXSSTV:
      mainWindowPtr->setSSTVDRMPushButton(false);
      ui->resyncToolButton->setEnabled(true);
      break;
    case TRXDRM:
      mainWindowPtr->setSSTVDRMPushButton(true);
      ui->resyncToolButton->setEnabled(false);
      break;
    default:
      break;
    }
  dispatcherPtr->startRX();
}

void rxWidget::slotLogCall()
{
  QString call;
  call=ui->callLineEdit->text().toUpper();
  dispatcherPtr->logSSTV(call,false);
}

void rxWidget::slotNewCall(QString call)
{
  ui->callLineEdit->setText(call);
  dispatcherPtr->logSSTV(call,true);
}

void rxWidget::slotResetCall()
{
  ui->callLineEdit->clear();
}

void rxWidget::slotErase()
{
  rxFunctionsPtr->eraseImage();
  imageViewerPtr->createImage(QSize(320,256),imageBackGroundColor,imageStretch);
}


void rxWidget::slotSave()
{
  QDateTime dt(QDateTime::currentDateTime().toUTC()); //this is compatible with QT 4.6
  QString path;
  QString info;
  dirDialog d(this);
  if(transmissionModeIndex==TRXSSTV)
    {
      path=rxSSTVImagesPath;

    }
  else
    {
      QMessageBox::information(this,"Saving image","Not available in DRM mode");
      return;
    }

  info="";
  QString fileName=d.saveFileName(path,"*","png");
  if (fileName.isNull()) return ;
  getImageViewerPtr()->save(fileName,defaultImageFormat,true,false);
  dispatcherPtr->saveImage(fileName,info);
}

void rxWidget::setOnlineStatus(bool online, QString info)
{
  QString fn;

  if(!online) dispatcherPtr->showOffLine();

  if(ff.isBusy())
    {
      return;
    }

  fn=myCallsign+"_"+info;
  //  ftpFunctions ff;
  // we can use onlineStatusInt directly because this function is only used from the main thread

  if(hybridFtpRemoteHost.isEmpty()) return;

  if(online && onlineStatusEnabled && transmissionModeIndex==TRXDRM)
    {
      ff.setupFtp("OnlineStatus",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin, hybridFtpPassword,hybridFtpRemoteDirectory+"/"+onlineStatusDir);
      ff.uploadData(QByteArray("Dummy\r\n"),fn,false,true);
    }
  else if(!online && transmissionModeIndex==TRXDRM)
    {
      displayMBoxEvent *stmb;
      stmb= new displayMBoxEvent("Hybrid Server","Cleaning up files on server");
      QApplication::postEvent( dispatcherPtr, stmb );  // Qt will delete it when done
      ff.setupFtp("OnlineStatus",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin, hybridFtpPassword,hybridFtpRemoteDirectory);


      if(!onlineStatusDir.isEmpty()&& onlineStatusEnabled)
        {
          ff.changePath(onlineStatusDir,true);
          ff.mremove("*"+myCallsign+"_*", true,false);
          ff.changePath("..",true);
        }
      if(!hybridFtpHybridFilesDirectory.isEmpty())
        {
          ff.changePath(hybridFtpHybridFilesDirectory,true);
          ff.mremove("*"+myCallsign+"-*", true,false);
          ff.changePath("..",true);
         }
       if(!hybridNotifyDir.isEmpty())
         {
          ff.changePath(hybridNotifyDir,true);
          ff.mremove("*"+myCallsign+"-*", true,true);
         }
    }
}



void rxWidget::slotWho()
{
  // get a list of online callsigns
  if(!ff.isBusy() && !onlineStatusDir.isEmpty())
    {
      ff.setupFtp("WhoResult",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin, hybridFtpPassword,hybridFtpRemoteDirectory+"/"+onlineStatusDir);
      ff.listFiles("*", true);
    }
  //slotWhoResult is called when we have the info
}

void rxWidget::slotWhoResult(bool err )
{
  int i;
  QString info;
  QDateTime lastModif;
  QDateTime now = QDateTime::currentDateTime();
  QList <QUrlInfo> users;

  if(err) return;

  users=ff.getListing();

  for(i=0;i<users.count();i++)
    {
      lastModif=users.at(i).lastModified();
      if(lastModif.secsTo(now)<=3600)
        {
          info += users.at(i).name()+"\n";
        }
    }
  ui->rxNotificationList->setPlainText(info);
}

void rxWidget::setSettingsTab()
{

  int i;
  if((transmissionModeIndex>=0)&&(transmissionModeIndex<TRXNOMODE))
    {
      for(i=0;i<TRXNOMODE;i++)
        {
          if(i!=transmissionModeIndex) ui->settingsTableWidget->widget(i)->setEnabled(false);
        }
      ui->settingsTableWidget->widget(transmissionModeIndex)->setEnabled(true);
      ui->settingsTableWidget->setCurrentIndex(transmissionModeIndex);
    }
  ui->vuWidget->setMaximum(100);
  ui->vuWidget->setMinimum(0);
  ui->vuWidget->setValue(-15);
  if(transmissionModeIndex==TRXDRM)
    {
      ui->syncWidget->setColors(QColor(0,90,0),QColor(0,190,0),Qt::green);
      ui->syncWidget->setMaximum(25.);
      ui->syncWidget->setMinimum(5.);
      ui->syncWidget->setValue(0.);
      ui->vuWidget->setColors(QColor(255,50,0),Qt::green,Qt::red);
      mainWindowPtr->spectrumFramePtr->displaySettings(true);
    }
  else
    {
      ui->syncWidget->setColors(Qt::red,QColor(255,165,0),Qt::green);
      ui->syncWidget->setMaximum(10.);
      ui->syncWidget->setMinimum(0.);
      ui->syncWidget->setValue(0.);
      ui->vuWidget->setColors(QColor(255,50,0),Qt::green,Qt::red);
      mainWindowPtr->spectrumFramePtr->displaySettings(false);
    }
}

void rxWidget::startRX(bool st)
{
  if(st)
    {
      getParams();
      dispatcherPtr->startRX();
      addToLog("starting rxfunction run",LOGRXMAIN);
    }
  else
    {
      dispatcherPtr->idleAll();
    }
}

bool rxWidget::rxBusy()
{
  return rxFunctionsPtr->rxBusy();
}

void rxWidget::setSSTVStatusText(QString txt)
{
  ui->sstvStatusLineEdit->setText(txt);
}


vuMeter *rxWidget::vMeterPtr()
{
  return ui->vuWidget;
}

vuMeter *rxWidget::sMeterPtr()
{
  return ui->syncWidget;
}

