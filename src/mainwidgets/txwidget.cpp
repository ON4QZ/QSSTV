#include "txwidget.h"
#include "appglobal.h"
#include "drmparams.h"
#include "rxwidget.h"
#include "rxfunctions.h"
#include "ui_txwidget.h"
#include <QTimer>
#include "configparams.h"
#include "dispatcher.h"
#include "supportfunctions.h"
#include "gallerywidget.h"
#include "waterfalltext.h"

#include "ui_freqform.h"
#include "ui_sweepform.h"
#include "cameradialog.h"
#include "soundbase.h"
#include "waterfallform.h"
#include "bsrform.h"
#include "mainwindow.h"
#include "rigcontrol.h"
#include "imageviewer.h"
#include "testpatternselection.h"


txWidget::txWidget(QWidget *parent) :  QWidget(parent), ui(new Ui::txWidget)
{
  int i;
  QString tmp;
  ui->setupUi(this);
  ui->previewWidget->setType(imageViewer::PREVIEW);
  txFunctionsPtr=new txFunctions(this);
  imageViewerPtr=ui->imageFrame;

  imageViewerPtr->displayImage();
  for(i=0;i<NUMSSTVMODES;i++)
    {
      ui->sstvModeComboBox->addItem(getSSTVModeNameLong((esstvMode)i));
    }
  sizeChanged=true;
  ui->sstvResizeComboBox->addItem("Stretch");
  ui->sstvResizeComboBox->addItem("Crop");
  ui->sstvResizeComboBox->addItem("Fit");

  connect(ui->sstvModeComboBox,SIGNAL(activated(int)),SLOT(slotModeChanged(int )));
  connect(ui->sstvResizeComboBox,SIGNAL(activated(int)),SLOT(slotResizeChanged(int)));

  connect(ui->drmTxBandwidthComboBox,SIGNAL(activated(int)),SLOT(slotGetTXParams()));
  connect(ui->drmTxInterleaveComboBox,SIGNAL(activated(int)),SLOT(slotGetTXParams()));
  connect(ui->drmTxProtectionComboBox,SIGNAL(activated(int)),SLOT(slotGetTXParams()));
  connect(ui->drmTxQAMComboBox,SIGNAL(activated(int)),SLOT(slotGetTXParams()));
  connect(ui->drmTxModeComboBox,SIGNAL(activated(int)),SLOT(slotGetTXParams()));
  connect(ui->drmTxReedSolomonComboBox,SIGNAL(activated(int)),SLOT(slotGetTXParams()));

  connect(ui->templatesComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotGetParams()));
  connect(ui->templateCheckBox,SIGNAL(toggled(bool)),SLOT(slotGetParams()));
  connect(ui->refreshPushButton,SIGNAL(clicked()),SLOT(slotGetParams()));
  connect(ui->cwCheckBox,SIGNAL(toggled(bool)),SLOT(slotGetTXParams()));
  connect(ui->voxCheckBox,SIGNAL(toggled(bool)),SLOT(slotGetTXParams()));
  connect(ui->toCallLineEdit,SIGNAL(editingFinished ()),SLOT(slotGetParams()));
  connect(ui->operatorLineEdit,SIGNAL(editingFinished ()),SLOT(slotGetParams()));
  connect(ui->rsvLineEdit,SIGNAL(editingFinished ()),SLOT(slotGetParams()));
  connect(ui->xPlainTextEdit,SIGNAL(editingFinished()),SLOT(slotGetParams()));
  connect(ui->yPlainTextEdit,SIGNAL(editingFinished()),SLOT(slotGetParams()));
  connect(ui->zPlainTextEdit,SIGNAL(editingFinished()),SLOT(slotGetParams()));


  connect(ui->startToolButton, SIGNAL(clicked()), this, SLOT(slotStart()));
  connect(ui->stopToolButton, SIGNAL(clicked()), this, SLOT(slotStop()));
  connect(ui->drmProfileComboBox,SIGNAL(activated(int)),SLOT(slotProfileChanged(int )));

  connect(ui->hybridCheckBox,SIGNAL(toggled(bool)),SLOT(slotHybridToggled()));
  connect(ui->uploadToolButton, SIGNAL(clicked()), this, SLOT(slotUpload()));

  connect(ui->generateToneToolButton, SIGNAL(clicked()), this, SLOT(slotGenerateSignal()));
  connect(ui->sweepToneToolButton, SIGNAL(clicked()), this, SLOT(slotSweepSignal()));
  connect(ui->repeaterToneToolButton, SIGNAL(clicked()), this, SLOT(slotGenerateRepeaterTone()));
  connect(ui->openToolButton, SIGNAL(clicked()), this, SLOT(slotFileOpen()));
  connect(ui->editToolButton, SIGNAL(clicked()), this, SLOT(slotEdit()));
  connect(ui->snapshotToolButton, SIGNAL(clicked()), this, SLOT(slotSnapshot()));
  connect(ui->binaryPushButton,SIGNAL(clicked()),this,SLOT(slotBinary()));
  connect(ui->sizeSlider,SIGNAL(valueChanged(int)),SLOT(slotSize(int)));
  connect(ui->settingsTableWidget,SIGNAL(currentChanged(int)),this, SLOT(slotTransmissionMode(int)));
  connect(imageViewerPtr,SIGNAL(imageChanged()),SLOT(slotImageChanged()));
  connect(ui->templateCheckBox,SIGNAL(toggled(bool)),SLOT(slotImageChanged()));
  connect(&notifyTimer,SIGNAL(timeout()),SLOT(slotNotifyTimeout()));
  connect(&ff,SIGNAL(listingDone(bool)),SLOT(slotListingDone(bool)));
  connect(&repeaterTxDelayTimer,SIGNAL(timeout()),SLOT(slotRepaterDelay()));
  maxSize=0;
  notifyTimer.setSingleShot(true);
  notifyTimer.setInterval(NOTIFYCHECKINTERVAL);
  repeaterTxDelayTimer.setSingleShot(true);

}

txWidget::~txWidget()
{
  writeSettings();
  txFunctionsPtr->terminate();
  delete txFunctionsPtr;

  delete ui;
}

void txWidget::init()
{
  splashStr+=QString( "Setting up TX" ).rightJustified(25,' ')+"\n";
  splashPtr->showMessage ( splashStr ,Qt::AlignLeft,Qt::white);
  qApp->processEvents();

  readSettings();
  initView();
  setProfileNames();
  ed=NULL;
  repeaterIndex=0;
  repeaterTimer=new QTimer(this);
  connect(repeaterTimer,SIGNAL(timeout()),SLOT(slotRepeaterTimer()));
  repeaterTimer->start(60000*repeaterImageInterval);
  addToLog("Reapater Timer Started",LOGTXMAIN);
  imageViewerPtr->setType(imageViewer::TXIMG);
  slotModeChanged(sstvModeIndexTx);
  changeTransmissionMode(transmissionModeIndex);
  //  setSettingsTab();
  slotProfileChanged(0);
  if(lowRes)
    {
      ui->txNotificationList->hide();
      ui->refreshPushButton->hide();
      ui->previewWidget->hide();
    }
}

void txWidget::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("TX");
  sstvModeIndexTx=((esstvMode)qSettings.value("sstvModeIndexTx",0).toInt());
  if((sstvModeIndexTx<M1) || (sstvModeIndexTx>=NOTVALID))
    {
      sstvModeIndexTx=M1;
    }

  templateIndex=qSettings.value("templateIndex",0).toInt();
  useTemplate=qSettings.value("useTemplate",false).toBool();

  useCW=qSettings.value("useCW",false).toBool();
  useVOX=qSettings.value("useVOX",false).toBool();
  useHybrid=qSettings.value("useHybrid",false).toBool();
  compressedSize=qSettings.value("compressedSize",5000).toUInt();
  drmParams.bandwith=qSettings.value("drmBandWith",0).toInt();
  drmParams.interleaver=qSettings.value("drmInterLeaver",0).toInt();
  drmParams.protection=qSettings.value("drmProtection",0).toInt();
  drmParams.qam=qSettings.value("drmQAM",0).toInt();
  drmParams.robMode=qSettings.value("drmRobMode",0).toInt();
  drmParams.reedSolomon=qSettings.value("drmReedSolomon",0).toInt();
  qSettings.endGroup();
  setParams();
}

void txWidget::writeSettings()
{
  QSettings qSettings;
  slotGetParams();
  qSettings.beginGroup("TX");
  qSettings.setValue( "sstvModeIndexTx", sstvModeIndexTx);
  qSettings.setValue( "templateIndex", templateIndex);
  qSettings.setValue( "useTemplate", useTemplate);
  qSettings.setValue( "useVOX", useVOX);
  qSettings.setValue( "useCW", useCW);
  qSettings.setValue( "useHybrid", useHybrid);
  qSettings.setValue("drmBandWith",drmParams.bandwith);
  qSettings.setValue("drmInterLeaver",drmParams.interleaver);
  qSettings.setValue("drmProtection",drmParams.protection);
  qSettings.setValue("drmQAM",drmParams.qam);
  qSettings.setValue("drmRobMode",drmParams.robMode);
  qSettings.setValue("drmReedSolomon",drmParams.reedSolomon);
  qSettings.setValue("compressedSize",compressedSize);
  qSettings.endGroup();
}

void txWidget::slotGetTXParams()
{
  // get only the params that don't require re-applying the template
  // used by prepareTX() and slotGetParams()
  int temp=sstvModeIndexTx;
  getIndex(temp,ui->sstvModeComboBox);
  sstvModeIndexTx=esstvMode(temp);
  getValue(useVOX,ui->voxCheckBox);
  getValue(useCW,ui->cwCheckBox);
  getIndex(drmParams.bandwith,ui->drmTxBandwidthComboBox);
  getIndex(drmParams.interleaver,ui->drmTxInterleaveComboBox);
  getIndex(drmParams.protection,ui->drmTxProtectionComboBox);
  getIndex(drmParams.qam,ui->drmTxQAMComboBox);
  getIndex(drmParams.robMode,ui->drmTxModeComboBox);
  getIndex(drmParams.reedSolomon,ui->drmTxReedSolomonComboBox);
  drmParams.callsign=myCallsign;
  updateTxTime();
  txFunctionsPtr->forgetTxFileName();
}


void txWidget::slotGetParams()
{
  getIndex(templateIndex,ui->templatesComboBox);
  getValue(useTemplate,ui->templateCheckBox);
  getValue(useHybrid,ui->hybridCheckBox);
  getValue(imageViewerPtr->toCall,ui->toCallLineEdit);
  getValue(imageViewerPtr->toOperator,ui->operatorLineEdit);
  getValue(imageViewerPtr->rsv,ui->rsvLineEdit);
  getValue(imageViewerPtr->comment1,ui->xPlainTextEdit);
  getValue(imageViewerPtr->comment2,ui->yPlainTextEdit);
  getValue(imageViewerPtr->comment3,ui->zPlainTextEdit);
  slotGetTXParams();
  getValue(compressedSize,ui->sizeSlider);
  ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
  if(txFunctionsPtr->getTXState()==txFunctions::TXIDLE)
    {
      applyTemplate();
    }
}

void txWidget::setParams()
{
  setIndex(((int)sstvModeIndexTx),ui->sstvModeComboBox);
  ui->templateCheckBox->blockSignals(true);
  ui->templatesComboBox->blockSignals(true);
  setIndex(templateIndex,ui->templatesComboBox);
  setValue(useTemplate,ui->templateCheckBox);
  ui->templateCheckBox->blockSignals(false);
  ui->templatesComboBox->blockSignals(false);
  setValue(useVOX,ui->voxCheckBox);
  setValue(useCW,ui->cwCheckBox);
  setValue(useHybrid,ui->hybridCheckBox);
  setIndex(drmParams.bandwith,ui->drmTxBandwidthComboBox);
  setIndex(drmParams.interleaver,ui->drmTxInterleaveComboBox);
  setIndex(drmParams.protection,ui->drmTxProtectionComboBox);
  setIndex(drmParams.qam,ui->drmTxQAMComboBox);
  setIndex(drmParams.robMode,ui->drmTxModeComboBox);
  setIndex(drmParams.reedSolomon,ui->drmTxReedSolomonComboBox);
  if(compressedSize<MINDRMSIZE) compressedSize=MINDRMSIZE;
  if(compressedSize>MAXDRMSIZE) compressedSize=MAXDRMSIZE;
  setValue(compressedSize,ui->sizeSlider);
  ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
  updateTxTime();
}

void txWidget::copyProfile(drmTxParams d)
{
  drmParams=d;
  setIndex(drmParams.bandwith,ui->drmTxBandwidthComboBox);
  setIndex(drmParams.interleaver,ui->drmTxInterleaveComboBox);
  setIndex(drmParams.protection,ui->drmTxProtectionComboBox);
  setIndex(drmParams.qam,ui->drmTxQAMComboBox);
  setIndex(drmParams.robMode,ui->drmTxModeComboBox);
  setIndex(drmParams.reedSolomon,ui->drmTxReedSolomonComboBox);
}

void txWidget::initView()
{
  imageViewerPtr->createImage(QSize(320,256),imageBackGroundColor,imageStretch);
  imageViewerPtr->setType(imageViewer::TXIMG);
  setupTemplatesComboBox();
  ui->txProgressBar->setRange(0,100);
}


void txWidget::setupTemplatesComboBox()
{
  QStringList sl;
  int i;
  ui->templatesComboBox->clear();
  sl=galleryWidgetPtr->getFilenames();
  for(i=0;i<sl.count();i++)
    {
      ui->templatesComboBox->insertItem(i,sl.at(i));
    }
  ui->templatesComboBox->setCurrentIndex(templateIndex);
}


QString txWidget::getPreviewFilename()
{
  return ui->previewWidget->getFilename();
}

void  txWidget::setPreviewWidget(QString fn)
{
  addToLog(QString("previewfile: %1").arg(fn),LOGTXMAIN);
  ui->previewWidget->openImage(fn,false,false,false,true);
}



void txWidget::slotStart()
{
  if(imageViewerPtr->hasValidImage())
    {
      doTx=TXNORMAL;
      prepareTx();
    }
  else
    {
      QMessageBox::warning(this,"TX Error","No image loaded");
    }
}

void txWidget::slotUpload()
{
  if(imageViewerPtr->hasValidImage())
    {
      doTx=TXUPLOAD;
      prepareTx();
    }
}


void txWidget::prepareTx()
{
  addToLog(QString("doTx=%1").arg(doTx),LOGTXMAIN);
  enableButtons(false);
  switch (transmissionModeIndex)
    {
    case TRXSSTV: txFunctionsPtr->prepareTX(txFunctions::TXPREPARESSTV);
      break;
    case TRXDRM:  txFunctionsPtr->prepareTX(txFunctions::TXPREPAREDRMPIC);
      break;
    case TRXNOMODE:
      break;
    }
}

void txWidget::prepareTxComplete(bool ok)
{
  addToLog(QString("ok=%1, doTx=%2").arg(ok).arg(doTx),LOGTXMAIN);
  if (!ok)
    {
      addToLog("Upload/prepare failed",LOGTXMAIN);
      ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
    }
  ui->startToolButton->setEnabled(true);
  if (ok && doTx!=TXUPLOAD)
    {
      if (doTx==TXBINARY)
        {
          if (useHybrid)
            {
              QMessageBox mbox(mainWindowPtr);
              QPushButton *sendButton;
              mbox.setWindowTitle("TX Binary File");
              mbox.setText("The file has been uploaded ready for transmission");

              sendButton = mbox.addButton(tr("Start Transmitting"), QMessageBox::AcceptRole);
              mbox.setStandardButtons(QMessageBox::Cancel);

              mbox.exec();
              if (mbox.clickedButton() == sendButton) {
                  dispatcherPtr->startTX(txFunctions::TXSENDDRMBINARY);
                }
            }
          else
            {
              dispatcherPtr->startTX(txFunctions::TXSENDDRMBINARY);
            }
        }
      else
        {
          startTxImage();
        }
    }
}

void txWidget::startTxImage()
{
  addToLog("startTxImage",LOGTXMAIN);
  QDateTime dt(QDateTime::currentDateTime().toUTC()); //this is compatible with QT 4.6
  dt.setTimeSpec(Qt::UTC);
  if(!imageViewerPtr->hasValidImage())
    {
      addToLog("invalidImage",LOGTXMAIN);
      return;
    }
  QFileInfo finf=imageViewerPtr->getFilename();
  QString fn;

  switch(transmissionModeIndex)
    {
    case TRXSSTV:
      if(saveTXimages)
        {
          fn=QString("%1/%2_%3.%4").arg(txSSTVImagesPath).arg(finf.baseName()).arg(dt.toString("yyyyMMdd_HHmmss")).arg(defaultImageFormat);
          imageViewerPtr->save(fn,defaultImageFormat,true,false);
          galleryWidgetPtr->txImageChanged();
        }
      addToLog("dispatch startTx",LOGTXMAIN);
      dispatcherPtr->startTX(txFunctions::TXSSTVIMAGE);
      break;
    case TRXDRM:
      if(saveTXimages)
        {
          fn=QString("%1/%2_%3.%4").arg(txDRMImagesPath).arg(finf.baseName()).arg(dt.toString("yyyyMMdd_HHmmss")).arg(defaultImageFormat);
          imageViewerPtr->save(fn,defaultImageFormat,true,false);
          galleryWidgetPtr->txImageChanged();
        }
      dispatcherPtr->startTX(txFunctions::TXSENDDRMPIC);
      break;
      //    case FAX:
      //    break;
    case TRXNOMODE:
      break;
    }
  ui->startToolButton->setEnabled(false);
}

void txWidget::sendBSR()
{
  QByteArray *p;
  bsrForm::eResult res;
  bsrForm bsrf;
  bsrf.init();
  res=(bsrForm::eResult)bsrf.exec();
  if(res==bsrForm::CANCEL) return;
  p=bsrf.getBA(res==bsrForm::COMPAT);
  txFunctionsPtr->sendBSR(p,bsrf.getDRMParams());
}


void txWidget::sendWFID()
{

  waterfallPtr->setText(myCallsign);
  dispatcherPtr->startTX(txFunctions::TXSENDWFID);
  addToLog("sendWFID",LOGTXMAIN);
}

void txWidget::sendCWID()
{
  dispatcherPtr->startTX(txFunctions::TXSENDCWID);
  addToLog("sendWFID",LOGTXMAIN);
}

void txWidget::sendWfText()
{
  waterfallForm wf;
  if((wf.exec()==QDialog::Accepted)&&(!wf.text().isEmpty()))
    {
      waterfallPtr->setText(wf.text());
      dispatcherPtr->startTX(txFunctions::TXSENDWFID);
      addToLog("sendID",LOGTXMAIN);
    }
}

void txWidget::slotStop()
{
  ui->startToolButton->setEnabled(true);
  enableButtons(true);
  dispatcherPtr->startRX();
}


void txWidget::enableButtons(bool enable)
{
  ui->uploadToolButton->setEnabled(enable && useHybrid && (transmissionModeIndex!=TRXSSTV));
  ui->hybridCheckBox->setEnabled(enable);
  ui->startToolButton->setEnabled(enable);
  ui->sstvModeComboBox->setEnabled(enable);
  ui->sstvResizeComboBox->setEnabled(enable);
  ui->voxCheckBox->setEnabled(enable);
  ui->cwCheckBox->setEnabled(enable);
  ui->templateCheckBox->setEnabled(enable);
  ui->templatesComboBox->setEnabled(enable);
  ui->refreshPushButton->setEnabled(enable);
}



void txWidget::slotFileOpen()
{
  QString fileName;
  imageViewerPtr->openImage(fileName,txStockImagesPath,true,true,true,false,true);
  applyTemplate();
}

void txWidget::slotGenerateSignal()
{
  QDialog qd;
  Ui::freqForm *ff=new Ui::freqForm;

  ff->setupUi(&qd);
  int freq;
  int  duration;
  if(qd.exec())
    {
      getValue(freq,ff->frequencySpinBox);
      getValue(duration,ff->durationSpinBox);
      txFunctionsPtr->setToneParam((double)duration,(double)freq);
      dispatcherPtr->startTX(txFunctions::TXSENDTONE);
    }
}

void txWidget::slotSweepSignal()
{
  QDialog qd;
  Ui::sweepForm *ff=new Ui::sweepForm;
  ff->setupUi(&qd);
  int upperFreq;
  int lowerFreq;
  int  duration;
  if(qd.exec())
    {
      getValue(lowerFreq,ff->lowerFrequencySpinBox);
      getValue(upperFreq,ff->upperFrequencySpinBox);
      getValue(duration,ff->durationSpinBox);
      txFunctionsPtr->setToneParam((double)duration,(double)lowerFreq,(double)upperFreq);
      dispatcherPtr->startTX(txFunctions::TXSENDTONE);
    }
}



void txWidget::slotGenerateRepeaterTone()
{
  //  addToLog(QString("start of buffer %1").arg(soundIOPtr->txBuffer.count()),LOGTXMAIN);
  txFunctionsPtr->setToneParam(3.,1750);
  dispatcherPtr->startTX(txFunctions::TXSENDTONE);
  addToLog("sendTone",LOGTXMAIN);
}




void txWidget::slotEdit()
{
  if (ed!=NULL) delete ed;
  ed=new editor(this);
  if(txFunctionsPtr->txBusy())
    {
      QMessageBox::warning(this,"Editor","Transmission busy, editor not available");
      return;
    }
//  connect(ed,SIGNAL(imageAvailable(QImage *)),SLOT(setImage(QImage *)));
  ed->setImage(imageViewerPtr->getImagePtr());
  ed->show();
}



void txWidget::applyTemplate()
{
  if(repeaterEnabled)
    {
      if(repeaterIdleImage)
        {
          txFunctionsPtr->applyTemplate(imageViewerPtr,repeaterIdleTemplate);
        }
      else
        {
          txFunctionsPtr->applyTemplate(imageViewerPtr,repeaterTemplate);
        }
    }
  else
    {
      txFunctionsPtr->applyTemplate(imageViewerPtr,galleryWidgetPtr->getTemplateFileName(ui->templatesComboBox->currentIndex()));
      ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
    }

  updateTxTime();

}

void txWidget::updateTxTime()
{
  int sizeKb,seconds,minutes;
  if(!imageViewerPtr->hasValidImage()) return;
  seconds=txFunctionsPtr->calcTxTime(false,0);
  minutes=seconds/60;
  seconds=seconds-minutes*60;
  if(minutes==0)
    {
      ui->sizeDurationLabel->setText(QString("%1 s").arg(seconds));
    }
  else
    {
      ui->sizeDurationLabel->setText(QString("%1 min %2 s").arg(minutes).arg(seconds));
    }
  sizeKb=round((double)imageViewerPtr->getFileSize()/1000.);
  ui->sizeKbLabel->setText( QString::number(sizeKb)+"KB");
}


void 	txWidget::setImage(QString fn)
{
  addToLog(QString("setImage %1").arg(fn),LOGTXMAIN);
  imageViewerPtr->openImage(fn,true,true,false,true);
  addToLog(QString("setImage %1 done").arg(fn),LOGTXMAIN);

}


void txWidget::setProgress(uint prg)
{
  ui->txProgressBar->setValue(prg);
}

void txWidget::slotModeChanged(int m)
{

  addToLog("slotModeChange",LOGTXMAIN);
  if(transmissionModeIndex==TRXSSTV)
    {
      sstvModeIndexTx=(esstvMode)m;
      applyTemplate();
    }
}

void txWidget::slotResizeChanged(int i)
{
  switch (i) {
    case 0: // Stretch
      imageViewerPtr->setAspectMode(Qt::IgnoreAspectRatio);
      break;
    case 1: // Crop
      imageViewerPtr->setAspectMode(Qt::KeepAspectRatioByExpanding);
      break;
    case 2: // Fit
      imageViewerPtr->setAspectMode(Qt::KeepAspectRatio);
      break;
    }
  applyTemplate();
}





void txWidget::slotSnapshot()
{
  QImage *im;
  cameraDialog camera;
  if(camera.exec()==QDialog::Accepted)
    {
      im=camera.getImage();
      if(im)
        {
          QTemporaryFile itmp(txStockImagesPath+"/snapshotXXXXXX.jpg");
          itmp.setAutoRemove(false);
          if(!itmp.open()) return;
          im->save(itmp.fileName());
          setImage(itmp.fileName());
          galleryWidgetPtr->txStockImageChanged();
        }
    }
}



void txWidget::setSettingsTab()
{
  {
    if(transmissionModeIndex==TRXDRM)
      {
        ui->hybridCheckBox->setEnabled(true);
        ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
        ui->binaryPushButton->setEnabled(true);
        ui->sizeLabel->setEnabled(true);
        ui->sizeSlider->setEnabled(true);
        ui->sizeKbLabel->setEnabled(true);
        ui->sizeDurationLabel->setEnabled(true);
        mainWindowPtr->setSSTVDRMPushButton(true);
      }
    else
      {
        ui->hybridCheckBox->setEnabled(false);
        ui->uploadToolButton->setEnabled(false);
        ui->binaryPushButton->setEnabled(false);
        ui->sizeLabel->setEnabled(false);
        ui->sizeSlider->setEnabled(false);
        ui->sizeKbLabel->setEnabled(false);
        ui->sizeDurationLabel->setEnabled(false);
        ui->sizeDurationLabel->setText("-");
        mainWindowPtr->setSSTVDRMPushButton(false);
      }
  }
  applyTemplate();
}


void txWidget::slotSize(int fsize)
{
  QString t;
  if((sizeChanged) || (compressedSize!=(uint)fsize))
    {
      sizeChanged=true;
      compressedSize=fsize;
      slotSizeApply();
    }
}

void txWidget::slotSizeApply()
{

  QApplication::setOverrideCursor(Qt::WaitCursor);
  sizeChanged=false;
  imageViewerPtr->setSize(compressedSize,transmissionModeIndex==TRXDRM);
  imageViewerPtr->displayImage();
  updateTxTime();
  QApplication::restoreOverrideCursor();
}

void txWidget::slotTransmissionMode(int rxtxMode)
{
  emit modeSwitch(rxtxMode);
}

void txWidget::changeTransmissionMode(int rxtxMode)
{
  int i;
  transmissionModeIndex=(etransmissionMode)rxtxMode;
  if((transmissionModeIndex>=0)&&(transmissionModeIndex<TRXNOMODE))
    {
      for(i=0;i<TRXNOMODE;i++)
        {
          if(i!=transmissionModeIndex) ui->settingsTableWidget->widget(i)->setEnabled(false);
        }
      ui->settingsTableWidget->widget(transmissionModeIndex)->setEnabled(true);
      ui->settingsTableWidget->blockSignals(true);
      ui->settingsTableWidget->setCurrentIndex(transmissionModeIndex);
      ui->settingsTableWidget->blockSignals(false);

    }
  setSettingsTab();
  if(transmissionModeIndex==TRXDRM)
    {
      slotSizeApply();
    }
}

void txWidget::slotProfileChanged(int i)
{
  drmProfileConfigPtr->getDRMParams(i,drmParams);
  setParams();
}

void txWidget::reloadProfiles()
{
  int index;
  index= ui->drmProfileComboBox->currentIndex();
  setProfileNames();
  slotProfileChanged(index);
}

void txWidget::setProfileNames()
{
  QString tmp;
  ui->drmProfileComboBox->clear();
  if(drmProfileConfigPtr->getName(0,tmp))
    {
      ui->drmProfileComboBox->addItem(tmp);
    }
  if(drmProfileConfigPtr->getName(1,tmp))
    {
      ui->drmProfileComboBox->addItem(tmp);
    }
  if(drmProfileConfigPtr->getName(2,tmp))
    {
      ui->drmProfileComboBox->addItem(tmp);
    }
}

void txWidget::slotImageChanged()
{
  uint temp;
  if(transmissionModeIndex==TRXDRM)
    {
      maxSize=imageViewerPtr->setSize(imageViewerPtr->diplayedImageBytecount(),transmissionModeIndex==TRXDRM);
      if(maxSize>MAXDRMSIZE) maxSize=MAXDRMSIZE;
      ui->sizeSlider->setMaximum(maxSize);
      ui->sizeSlider->setMinimum(MINDRMSIZE);
      if(compressedSize>MAXDRMSIZE) compressedSize=MAXDRMSIZE;
      if(compressedSize<MINDRMSIZE) compressedSize=MINDRMSIZE;
      temp=compressedSize;
      compressedSize=0;
      slotSize(temp);
    }
  else
    {
      slotSizeApply();
    }
  ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
  updateTxTime();
}

void txWidget::slotBinary()
{
  slotGetParams();
  doTx=TXBINARY;
  dispatcherPtr->startDRMTxBinary();
}

void txWidget::slotHybridToggled()
{
  getValue(useHybrid,ui->hybridCheckBox);
  ui->uploadToolButton->setEnabled(useHybrid && (transmissionModeIndex!=TRXSSTV));
  updateTxTime();
}


void txWidget::txTestPattern(etpSelect sel)
{
  txFunctionsPtr->txTestPattern(imageViewerPtr,sel);
}

void txWidget::startNotifyCheck(QString fn)
{
  mask="Dummy"+fn+"+++*";
  numberOfNotifyChecks=NUMBEROFNOTIFYCHECKS;
  notifyTimer.start();
  notifyBusy=false;
}

void txWidget::slotNotifyTimeout()
{
  if(notifyBusy)
    {
      numberOfNotifyChecks--;
      if(numberOfNotifyChecks>=0)
        {
          notifyTimer.start();
        }
      return;
    }
  notifyBusy=true;
  ff.setupFtp("Notify check", hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin,hybridFtpPassword,hybridFtpRemoteDirectory+"/"+hybridNotifyDir);
  ff.listFiles(mask,true);
}

void txWidget::slotListingDone(bool err)
{
  int i;
  QString info;
  QString tmp;
  QList <QUrlInfo> notifications;
  if(err) return;
  notifications=ff.getListing();

  for(i=0;i<notifications.count();i++)
    {
      tmp=notifications.at(i).name();
      tmp.replace("Dummyde_","");
      tmp.replace("+++."," ");
      info += tmp;
    }
  ui->txNotificationList->setPlainText(info);
  numberOfNotifyChecks--;
  if(numberOfNotifyChecks>=0)
    {
      notifyTimer.start();
    }
  notifyBusy=false;
}

void txWidget::sendRepeaterImage(esstvMode rxMode)
{
  useHybrid=0;
  setValue(useHybrid,ui->hybridCheckBox);
  rxWidgetPtr->getImageViewerPtr()->slotToTX();
  repeaterTxDelayTimer.start(repeaterTxDelay*1000);

  if(transmissionModeIndex==TRXSSTV)
    {
      if(repeaterTxMode==0) txMode=rxMode; // 0 index = Same as RX
      else txMode=(esstvMode)(repeaterTxMode-1);
      slotModeChanged(txMode);
    }
}


void txWidget::slotRepaterDelay()
{
  addToLog("repeater send image",LOGTXMAIN);
//  qDebug() << "starting repeater tx";
  repeaterIdleImage=false;
  repeaterTimer->start(60000*repeaterImageInterval);  //retrigger repeater timer for idle images
  //
  // imPtr points to image to send
  doTx=TXNORMAL;
  slotSize(repeaterImageSize*1000);
  prepareTx();
}





void txWidget::slotRepeaterTimer()
{
  QString fn;
  QFile fi;
  repeaterTimer->start(60000*repeaterImageInterval); //retrigger repeater timer
  if( txFunctionsPtr->txBusy()
      || rxWidgetPtr->rxBusy()
      || !repeaterEnabled )
    {
      return;
    }

  switch(repeaterIndex)
    {
    case 0:
      fn=repeaterImage1;
      break;
    case 1:
      fn=repeaterImage2;
      break;
    case 2:
      fn=repeaterImage3;
      break;
    case 3:
      fn=repeaterImage4;
      break;
    default:
      fn=repeaterImage1;
      break;
    }
  fi.setFileName(fn);
  if (fi.exists())
    {
      slotModeChanged(repeaterIdleTxMode);
      setImage(fn);
      repeaterIdleImage=true;
      doTx=TXNORMAL;
      addToLog("repeater send idle image",LOGTXMAIN);
      slotSize(repeaterImageSize*1000);
      prepareTx();
    }
  repeaterIndex++;
  if(repeaterIndex>3) repeaterIndex=0;
}












