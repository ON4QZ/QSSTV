/***************************************************************************
 *   Copyright (C) 2000-2019 by Johan Maes                                 *
 *   on4qz@telenet.be                                                      *
 *   http://users.telenet.be/on4qz                                         *
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

/*!
  The dispatcher is the central system that routes all messages from the different threads.
It also starts, stops and synchronizes the threads.

*/
#include "dispatcher.h"
#include "appglobal.h"
#include "configparams.h"
#include "rxwidget.h"
#include "txwidget.h"
#include "gallerywidget.h"
#include "spectrumwidget.h"
#include "vumeter.h"
#include "rxfunctions.h"
#include "mainwindow.h"
#include "rigcontrol.h"
#include "logbook.h"
#include "dirdialog.h"
#include "ftpfunctions.h"

#include <QSettings>
#include <QMessageBox>

/*! creates dispatcher instance  */

dispatcher::dispatcher()
{
  mbox=NULL;
  progressFTP=NULL;
  lastFileName.clear();
  prTimerIndex=0;
}

/*! delete dispatcher instance  */

dispatcher::~dispatcher()
{
}

void dispatcher::init()
{
  editorActive=false;
  infoTextPtr=new textDisplay(mainWindowPtr);
  mainWindowPtr->spectrumFramePtr->init(RXSTRIPE,fftNumBlocks,BASESAMPLERATE/SUBSAMPLINGFACTOR);
  infoTextPtr->hide();
  connect(&ff,SIGNAL(listingDone(bool)),SLOT(slotRenameListing(bool)));
}


/*!
  All communication between the threads are passed via this eventhandler.
*/

void dispatcher::customEvent( QEvent * e )
{
  dispatchEventType type;
  QString fn;
  type=(dispatchEventType)e->type();
  if((type!=displayFFT)
     && (type!=displaySync)
     && (type!=rxSSTVStatus)
     && (type!=lineDisplay)
     && (type!=displayDRMInfo)
     && (type!=displayDRMStat)
     )
    {
      addToLog(((baseEvent*)e)->description,LOGDISPATCH);
    }
  switch(type)
    {
    case displayFFT:
      mainWindowPtr->spectrumFramePtr->realFFT(((displayFFTEvent*)e)->data());
      rxWidgetPtr->vMeterPtr()->setValue(soundIOPtr->getVolumeDb());
      break;
    case displaySync:
      uint s;
      ((displaySyncEvent*)e)->getInfo(s);
      rxWidgetPtr->sMeterPtr()->setValue((double)s);
      break;
    case rxSSTVStatus:
      rxWidgetPtr->setSSTVStatusText(((rxSSTVStatusEvent*)e)->getStr());
      break;

    case startImageRX:
      addToLog("dispatcher: clearing RxImage",LOGDISPATCH);
      //      rxWidgetPtr->getImageViewerPtr()->createImage( ((startImageRXEvent*)e)->getSize(),QColor(0,0,128),imageStretch);
      rxWidgetPtr->getImageViewerPtr()->createImage( ((startImageRXEvent*)e)->getSize(),imageBackGroundColor,imageStretch);
      lastCallsign="";
      break;
    case lineDisplay:
      {
        rxWidgetPtr->getImageViewerPtr()->displayImage();
      }
      break;
    case endSSTVImageRX:
      if(autoSave)
        {
          addToLog("dispatcher:endImage savingRxImage",LOGDISPATCH);
          saveRxSSTVImage(((endImageSSTVRXEvent*)e)->getMode());
        }
      break;


    case rxDRMStatus:
      rxWidgetPtr->setDRMStatusText(((rxDRMStatusEvent*)e)->getStr());

      break;

    case statusBarMsg:
      statusBarPtr->showMessage(((statusBarMsgEvent*)e)->getStr());
      break;
    case callEditor:
      if(editorActive) break;
      editorActive=true;
      ed=new editor();
      ed->show();
      iv=((callEditorEvent*)e)->getImageViewer();
      addToLog (QString(" callEditorEvent imageViewPtr: %1").arg(QString::number((ulong)iv,16)),LOGDISPATCH);
      addToLog(QString("editor: filename %1").arg(((callEditorEvent*)e)->getFilename()),LOGDISPATCH);
      ed->openFile(((callEditorEvent*)e)->getFilename());
      break;

    case txDRMNotify:
      txWidgetPtr->setDRMNotifyText(((txDRMNotifyEvent*)e)->getStr());
      break;
    case txDRMNotifyAppend:
      txWidgetPtr->appendDRMNotifyText(((txDRMNotifyAppendEvent*)e)->getStr());
      break;
    case txPrepareComplete:
      txWidgetPtr->prepareTxComplete(((txPrepareCompleteEvent *)e)->ok());
      break;

    case editorFinished:
      if(!editorActive) break;
      if(((editorFinishedEvent*)e)->isOK())
        {
          addToLog (QString(" editorFinishedEvent imageViewPtr: %1").arg(QString::number((ulong)iv,16)),LOGDISPATCH);
          iv->reload();
        }
      editorActive=false;
      delete ed;
      break;

    case templatesChanged:
      galleryWidgetPtr->changedMatrix(imageViewer::TEMPLATETHUMB);
      txWidgetPtr->setupTemplatesComboBox();
      break;

    case progressTX:
      txTimeCounter=0;
      addToLog(QString("dispatcher: progress duration=%1").arg(((progressTXEvent*)e)->getInfo()),LOGDISPATCH);
      prTimerIndex=startTimer(((progressTXEvent*)e)->getInfo()*10); // time in seconds -> times 1000 for msec,divide by 100 for progress
      break;

    case stoppingTX:
      addToLog("dispatcher: endTXImage",LOGDISPATCH);
      break;

    case endImageTX:
      //addToLog("dispatcher: endTXImage",LOGDISPATCH);
      while(soundIOPtr->isPlaying())
        {
          qApp->processEvents();
        }
      addToLog("dispatcher: endTXImage",LOGDISPATCH);
      txWidgetPtr->slotStop();
//      startRX();
      break;

    case displayDRMInfo:
      if(!slowCPU)
        {
          rxWidgetPtr->mscWdg()->setConstellation(MSC);
          rxWidgetPtr->facWdg()->setConstellation(FAC);
        }
      rxWidgetPtr->statusWdg()->setStatus();
      break;

    case displayDRMStat:
      DSPFLOAT s1;
      ((displayDRMStatEvent*)e)->getInfo(s1);
      rxWidgetPtr->sMeterPtr()->setValue(s1);
      break;

    case loadRXImage:
      {
        QString fn=((loadRXImageEvent *)e)->getFilename();
        rxWidgetPtr->getImageViewerPtr()->openImage(fn,false,false,false,true);
      }
      break;
    case moveToTx:
      {
        addToLog(QString("moveToTx: %1").arg(((moveToTxEvent *)e)->getFilename()),LOGDISPATCH);
        txWidgetPtr->setImage(((moveToTxEvent *)e)->getFilename());
      }
      break;
    case saveDRMImage:
      {
        QString info;
        ((saveDRMImageEvent*)e)->getFilename(fn);
        ((saveDRMImageEvent*)e)->getInfo(info);
        if(!rxWidgetPtr->getImageViewerPtr()->openImage(fn,false,false,false,false))
          {
            // simply save the file if it is not an image file
            if(mbox==NULL) delete mbox;
            mbox = new QMessageBox(mainWindowPtr);
            mbox->setWindowTitle("Received file");
            mbox->setText(QString("Saved file %1").arg(fn));
            mbox->show();
            QTimer::singleShot(4000, mbox, SLOT(hide()));
            break;
          }
        saveImage(fn, info);
        if(repeaterEnabled)
          {
            txWidgetPtr->sendRepeaterImage();
          }
      }
      break;

    case prepareFix:
      addToLog("prepareFix",LOGDISPATCH);
      startDRMFIXTx( ((prepareFixEvent*)e)->getData());
      break;
    case displayText:
      infoTextPtr->clear();
      infoTextPtr->setWindowTitle(QString("Received from %1").arg(drmCallsign));
      infoTextPtr->append(((displayTextEvent*)e)->getStr());
      infoTextPtr->show();
      break;

    case displayMBox:
      if(mbox==NULL) delete mbox;
      mbox = new QMessageBox(mainWindowPtr);
      mbox->setWindowTitle(((displayMBoxEvent*)e)->getTitle());
      mbox->setText(((displayMBoxEvent*)e)->getStr());
      mbox->show();
      QTimer::singleShot(4000, mbox, SLOT(hide()));
      break;

    case displayProgressFTP:
      {
        if(((displayProgressFTPEvent*)e)->getTotal()==0)
          {
            delete progressFTP;
            progressFTP=NULL;
            break;
          }
        if(progressFTP==NULL)
          {
            progressFTP=new QProgressDialog("FTP Transfer","Cancel",0,0,mainWindowPtr);
          }
        progressFTP->show();
        progressFTP->setMaximum(((displayProgressFTPEvent*)e)->getTotal());
        progressFTP->setValue(((displayProgressFTPEvent*)e)->getBytes());
      }
      break;
//    case notifyAction: // sends notification after reception of hybrid image
//      // todo     notifyRXIntfPtr->mremove(((notifyActionEvent*)e)->getToRemove());
//      //      notifyRXIntfPtr->uploadData(((notifyActionEvent*)e)->getMsg().toLatin1(), ((notifyActionEvent*)e)->getFilename());
//      break;

    case notifyCheck:
      txWidgetPtr->startNotifyCheck(((notifyCheckEvent*)e)->getFilename());
      break;
    default:
      addToLog(QString("unsupported event: %1").arg(((baseEvent*)e)->description), LOGALL);
      break;
    }
  ((baseEvent *)e)->setDone();
}


void dispatcher::idleAll()
{
  if(prTimerIndex>=0)
    {
      killTimer(prTimerIndex);
      prTimerIndex=-1;
      txWidgetPtr->setProgress(0);
    }
  rigControllerPtr->activatePTT(false);
  rxWidgetPtr->functionsPtr()->stopAndWait();
  txWidgetPtr->functionsPtr()->stopAndWait();
}


void dispatcher::startRX()
{
  idleAll();
  soundIOPtr->startCapture();
  rxWidgetPtr->functionsPtr()->startRX();
}

void dispatcher::startTX(txFunctions::etxState state)
{
  idleAll();
  rigControllerPtr->activatePTT(true);
  soundIOPtr->startPlayback();
  txWidgetPtr->functionsPtr()->startTX(state);
}


void dispatcher::startDRMFIXTx(QByteArray ba)
{
  if(!txWidgetPtr->functionsPtr()->prepareFIX(ba)) return;
  startTX(txFunctions::TXSENDDRMFIX);
}

void dispatcher::startDRMTxBinary()
{
  //TODO: this whole thing should probably live in txWidget::slotBinary
  QFileInfo finfo;
  int sizeOfFile;
  QString sizeStr;
  int txtime=0;
  QMessageBox mbox(mainWindowPtr);
  QPushButton *sendButton;

  dirDialog d((QWidget *)mainWindowPtr,"Binary File");
  QString filename=d.openFileName("","*");
  if(filename.isEmpty()) return;
  if(!txWidgetPtr->functionsPtr()->prepareBinary(filename)) return;

  txtime = txWidgetPtr->functionsPtr()->calcTxTime(true,0);
  finfo.setFile(filename);

  if (txtime > (3*60))
    mbox.setIcon(QMessageBox::Warning);

  mbox.setWindowTitle("TX Binary File");
  mbox.setText(QString("'%1'").arg(filename));
  sizeOfFile=finfo.size();
  if(sizeOfFile<1000)
    {
      sizeStr=QString("%1 bytes").arg(sizeOfFile);
    }
  else if(sizeOfFile<10000)
    {
      sizeStr=QString("%1 kB").arg(sizeOfFile/1000.0,3,'f',2);
    }
  else
    {
      sizeStr=QString("%1 kB").arg(sizeOfFile/1000.0,3,'f',1);
    }

  mbox.setInformativeText(QString("The file is %1 and will take %2 seconds on air to send").
                          arg(sizeStr).arg(txtime));

  if (useHybrid)
    sendButton = mbox.addButton(tr("Upload ready to transmit"), QMessageBox::AcceptRole);
  else
    sendButton = mbox.addButton(tr("Start Transmitting"), QMessageBox::AcceptRole);
  mbox.setStandardButtons(QMessageBox::Cancel);

  mbox.exec();
  if (mbox.clickedButton() == sendButton) {
      txWidgetPtr->functionsPtr()->prepareTX(txFunctions::TXPREPAREDRMBINARY);
    }
}



void dispatcher::logSSTV(QString call,bool fromFSKID)
{
  if(lastFileName.isEmpty())
    {
      return;
    }
  if(fromFSKID)
    {
      QDateTime dt(QDateTime::currentDateTime().toUTC());
      int diffsec=saveTimeStamp.secsTo(dt);
      if(diffsec<2)
        {
          logBookPtr->logQSO(call,"SSTV",lastFileName);
        }
      lastCallsign=call;
    }
  else
    {
      logBookPtr->logQSO(call,"SSTV","");
    }

}


void dispatcher::saveRxSSTVImage(esstvMode mode)
{
  QString info,s,fileName;
  QString shortModeName=getSSTVModeNameShort(mode);

  QTemporaryFile tfn;
  int m;
  QDateTime dt(QDateTime::currentDateTime().toUTC()); //this is compatible with QT 4.6
  dt.setTimeSpec(Qt::UTC);
  if (mode==NOTVALID)
    {
      lastFileName.clear();
      return;
    }
  if(repeaterEnabled)
    {
      txWidgetPtr->sendRepeaterImage(mode);
    }
  if(!autoSave)
    {
      lastFileName=shortModeName;
    }
  else
    {
      fileName=QString("%1/%2_%3.%4").arg(rxSSTVImagesPath).arg(shortModeName).arg(dt.toString("yyyyMMdd_HHmmss")).arg(defaultImageFormat);
      addToLog(QString("dispatcher: saveRxImage():%1 ").arg(fileName),LOGDISPATCH);
      rxWidgetPtr->getImageViewerPtr()->save(fileName,defaultImageFormat,true,false);

      info="";
      m=0;
      while (m<=NUMSSTVMODES && shortModeName!=SSTVTable[m].shortName) m++;
      if (m<=NUMSSTVMODES)
        info += SSTVTable[m].name;
      else
        info += shortModeName;

      if (!lastCallsign.isEmpty())
        info += " de "+lastCallsign;

      saveImage(fileName, info);
      lastFileName=QString("%1_%2.%3").arg(shortModeName).arg(dt.toString("yyyyMMdd_HHmmss")).arg(defaultImageFormat);
      saveTimeStamp= dt;
    }
}

void dispatcher::saveImage(QString fileName, QString infotext)
{
  // filename is the name of the original file in hybrid mode
  QFileInfo info(fileName);
  QString fn="/tmp/"+info.baseName()+"."+ftpDefaultImageFormat;
  galleryWidgetPtr->putRxImage(fileName);
  txWidgetPtr->setPreviewWidget(fileName);
  if(enableFTP)
    {
      QImage *imp = rxWidgetPtr->getImageViewerPtr()->getImagePtr();
      QImage im;

      if (imp && imp->width())
        {
          // the original source image is available, possibly in higher quality
          // than the displayed image. Only for DRM Images.
          im = QImage(imp->convertToFormat(QImage::Format_RGB32));
        }
      else {
          // Uses the displayed image in whatever quality is displayed.
          // Applies to SSTV images.
          rxWidgetPtr->getImageViewerPtr()->save(fn,ftpDefaultImageFormat,true,false);
          im = QImage(fn);
        }
      QString text, remoteDir;
      QPainter p;
      double freq=0;
      int pixelSize, height, width;
      rigControllerPtr->getFrequency(freq);
      text = QString("%1 UTC %2 kHz ").arg(QDateTime::currentDateTime().toUTC().toString("hh:mm ddd MMM d, yyyy")).arg(freq/1000,1,'f',0);

      if (transmissionModeIndex==TRXSSTV)
        {
          remoteDir = ftpRemoteSSTVDirectory;
        }
      else
        {
          remoteDir = ftpRemoteDRMDirectory;
        }
      if (!infotext.isEmpty()) text += " "+infotext;
      // Limit uploaded size
      if ((im.width() > 960) || (im.height() > 768)) {
          im = im.scaled(960,768, Qt::KeepAspectRatio);
        }
      // Stamp text over the top left of the image
      // and keep it the same portion, unless the
      // font would be unreadable
      QFont font("Arial");
      pixelSize = 9 * im.width()/320;
      if (pixelSize<8) pixelSize=8;
      font.setPixelSize(pixelSize);
      QFontMetrics fontm(font);
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
      width = fontm.width(text) + 6;
#else
      width = fontm.horizontalAdvance(text) + 6;
#endif
      height= fontm.height() + 2;
      p.begin(&im);
      p.setPen(Qt::black);
      p.fillRect(0,0,width,height, Qt::black);
      p.setPen(Qt::white);
      p.setBrush(Qt::white);
      p.setFont(font);
      p.drawText(2,height-fontm.descent()-1, text);
      im.save( fn, ftpDefaultImageFormat.toUpper().toLatin1().data());
      p.end();
      uploadToRXServer(remoteDir, fn);
    }
}

void dispatcher::uploadToRXServer(QString remoteDir, QString fn)
{
  // todo ftp
  QString uploadDestinationFile=fn;
  uploadSourceFile=fn;
  if(ff.isBusy()) return;
  ff.setupFtp("FTP Upload to server",ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,remoteDir);

  QImage im(1,QDateTime::currentDateTime().time().minute(),QImage::Format_RGB32);
  im.fill(Qt::black);
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  im.save(&buffer, "JPG");
  ff.uploadData(ba,"checknew.jpg",false,false);
  if(ftpSaveFormat==FTPIMAGESEQUENCE)
    {
      ff.listFiles("image*",false);
    }
  else
    {
      ff.uploadFile(uploadSourceFile,uploadDestinationFile,false,true);
    }
}


void dispatcher::slotRenameListing(bool err)
{
  int i;
  if(err) return;
  QList <QUrlInfo> images;
  QString extension="";

  if(addExtension) extension="."+ftpDefaultImageFormat.toLower();
  images=ff.getListing();
  QString src,dst;
  for(i=ftpNumImages;i>1;i--)
    {
      src=QString("image%1%2").arg(i-1).arg(extension);
      dst=QString("image%1%2").arg(i).arg(extension);
      if(inList(images,src))
        {
          ff.rename(src,dst,false,false);
        }
    }
  ff.uploadFile(uploadSourceFile,"image1"+extension,false,true);
}



bool dispatcher::inList(QList <QUrlInfo> lst,QString fn)
{
  int i;
  for(i=0;i<lst.count();i++)
    {
      if(lst.at(i).name()==fn) return true;
    }
  return false;
}



void dispatcher::showOffLine()
{
  if(!enableFTP) return;
  ff.setupFtp("FTP Show Offline",ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,ftpRemoteSSTVDirectory);
  QImage im(2,QDateTime::currentDateTime().time().minute(),QImage::Format_RGB32);
  im.fill(Qt::black);
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  im.save(&buffer, "JPG");
  ff.uploadData(ba,"checknew.jpg",true,false);
  ff.changePath("/"+ftpRemoteDRMDirectory,true);
  ff.uploadData(ba,"checknew.jpg",true,true);
}


void dispatcher::timerEvent(QTimerEvent *event)
{
  if(event->timerId()==prTimerIndex)
    {
      txWidgetPtr->setProgress(++txTimeCounter);
      if(txTimeCounter>=100)
        {
          if(prTimerIndex>=0)
            {
              killTimer(prTimerIndex);
              prTimerIndex=-1;
              txWidgetPtr->setProgress(0);
            }
        }
      txWidgetPtr->setProgress(txTimeCounter);
    }
}



void dispatcher::slotTXNotification(QString info)
{
  if (info != "")
    {
      txWidgetPtr->appendDRMNotifyText(info);
    }
}
