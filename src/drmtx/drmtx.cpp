#include "drmtx.h"
#include "appglobal.h"
#include "soundbase.h"
#include "dispatcher.h"
#include "drmtransmitter.h"
#include "drmparams.h"
#include "reedsolomoncoder.h"
#include "hybridcrypt.h"
#include "configparams.h"
#include "txwidget.h"

#include "hybridcrypt.h"


drmTx::drmTx(QObject *parent) :
  QObject(parent)
{
  txDRM=new drmTransmitter;
  txList.clear();

  hybridTxCount=1;
}

drmTx::~drmTx()
{
  delete txDRM;

}

void drmTx::init()
{

}

void drmTx::start()
{
  statusBarMsgEvent *statBar;
  statBar=new statusBarMsgEvent("Sending "+drmTxFileName.fileName());
  QApplication::postEvent(dispatcherPtr,statBar);
  txDRM->start(true);


}

void drmTx::forgetTxFileName()
{
  drmTxStamp="";
}

QString drmTx::getTxFileName(QString fileName)
{
  QString prefix;
  QFileInfo finf;

  if(useHybrid)
    {
      prefix=QString("de_"+myCallsign+"-%1-").arg(hybridTxCount);
    }
  else {
      // Once allocated, keep the same TX name...
      // cleared by applyTemplate() or forgetTxFileName()
      if (drmTxStamp.isEmpty())
        drmTxStamp=QDateTime::currentDateTime().toUTC().toString("yyyyMMddHHmmss");
      prefix = drmTxStamp+"-";
    }

  if(fileName.isEmpty())
    {
      finf.setFile(txWidgetPtr->getImageViewerPtr()->getCompressedFilename());
    }
  else
    {
      finf.setFile(fileName);
    }

  drmTxFileName.setFile(prefix + finf.fileName());
  return drmTxFileName.fileName();
}

bool drmTx::initDRMImage(bool binary,QString fileName)
{
  eRSType rsType;
  reedSolomonCoder rsd;
  QFile inf;
  setTxParams(drmParams);

  if (binary && fileName.isEmpty()) return false;
  if (!binary && !txWidgetPtr->getImageViewerPtr()->hasValidImage()) return false;

  addToLog(QString("bin=%1, fileName=%2").arg(binary).arg(fileName),LOGDRMTX);

  // make sure drmTxFileName is up to date
  getTxFileName(fileName);

  if(!useHybrid)
    {
      if(binary)
        {
          inf.setFileName(fileName);
          if(!inf.open(QIODevice::ReadOnly))
            {
              addToLog(QString("Unable to open '%1' for read").arg(fileName),LOGDRMTX);
              return false;
            }
          baDRM=inf.readAll();
        }
      else
        {
          if(!txWidgetPtr->getImageViewerPtr()->copyToBuffer(&baDRM))
            {
              addToLog(QString("Unable to copy image from txWidget->viewer"),LOGDRMTX);
              return false;
            }
          drmTxFileName.setFile(drmTxFileName.completeBaseName()+".jp2");
        }

      rsType=(eRSType)drmTxParameters.reedSolomon;
      if(rsType!=RSTNONE)
        {
          rsd.encode(baDRM,drmTxFileName.suffix(),rsType);
          txDRM->init(&baDRM,drmTxFileName.baseName(),rsTypeStr[rsType],drmTxParameters);
        }
      else
        {
          txDRM->init(&baDRM,drmTxFileName.baseName(),drmTxFileName.suffix(),drmTxParameters);
        }
    }
  else
    {
      hybridCrypt hc;
      drmTxHybridParameters.bandwith=1; // bw 2.2
      drmTxHybridParameters.robMode=2;  // mode E
      drmTxHybridParameters.interleaver=0; // long
      drmTxHybridParameters.protection=0; // high
      drmTxHybridParameters.qam=0; // 4bit QAM
      drmTxHybridParameters.callsign=myCallsign;

      // we have to fill in the body
      baDRM.clear();
      hc.enCrypt(&baDRM);
      txDRM->init(&baDRM,drmTxFileName.baseName(),drmTxFileName.suffix(),drmTxHybridParameters);
      hybridTxCount++;
    }
  return true;
}

void drmTx::updateTxList()
{
  fixBlockList.clear();
  if ((txList.count()>0) && (txList.last().transportID == txTransportID)) return;

  if(txList.count()>5) txList.removeFirst();

  txList.append(txSession());
  txList.last().filename=drmTxFileName.baseName();
  txList.last().extension=drmTxFileName.suffix();
  txList.last().ba=baDRM;

  if (useHybrid)
    {
      txList.last().drmParams=drmTxHybridParameters;
      ftpDRMHybridNotifyCheck(txList.last().filename+"."+txList.last().extension);
    }
  else
    {
      txList.last().drmParams=drmTxParameters;
    }

  // transportID is set
  txList.last().transportID=txTransportID;
}

bool drmTx::ftpDRMHybrid(QString fileName, QString destName)
{
  QByteArray ba;
  //  ftpFunctions ff;
  //  QString ftpErrorStr;
  addToLog(QString("destName='%1' last='%2'").arg(destName).arg(lastHybridUpload),LOGFTPFUNC);

  if (destName == lastHybridUpload)
    {
      addToLog(QString("%1 already uploaded").arg(destName), LOGFTPFUNC);
      return true;
    }

  if (fileName.isEmpty())
    {
      txWidgetPtr->getImageViewerPtr()->copyToBuffer(&ba);
      if(!ftmp.open()) return false;
      ftmp.write(ba);
      ftmp.close();
      fileName=ftmp.fileName();
    }
  ff.setupFtp("UploadHybridFile",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin,hybridFtpPassword,hybridFtpRemoteDirectory+"/"+hybridFtpHybridFilesDirectory);
  ff.uploadFile(fileName,destName,false,true);
  lastHybridUpload=destName;
  addToLog(QString("hybridTxCount updated to %1").arg(hybridTxCount),LOGDRMTX);
  return true;
}

void drmTx::clearLastHybridUpload()
{
  if(!lastHybridUpload.isEmpty())
    {
      addToLog(QString("Clearing lasHybridUpload, was: %1").arg(lastHybridUpload),LOGFTPFUNC);
      lastHybridUpload = "";
    }
}

void drmTx::ftpDRMHybridNotifyCheck(QString mask)
{
  if (!enableHybridNotify) return;
  notifyCheckEvent *ce;
  ce=new notifyCheckEvent(mask);
  QApplication::postEvent( dispatcherPtr, ce );
}


double  drmTx::calcTxTime(int overheadTime)
{
  double tim=0;
  //  tim= soundIOPtr->getPlaybackStartupTime();
  tim+=overheadTime;
  tim+=txDRM->getDuration();
  return tim;
}

int drmTx::processFIX(QByteArray bsrByteArray)
{
  int i,j;
  bool inSeries;
  //  bool extended; // todo check use of extended
  bool done;
  int block;
  int trID,lastBlock;

  fixBlockList.clear();
  QString str(bsrByteArray);
  str.replace("\r","");
  //  information is in the QByteArray ba
  QStringList sl;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
         sl=str.split("\n",Qt::SkipEmptyParts);

#else
        sl=str.split("\n",QString::SkipEmptyParts);
#endif


  if(sl.at(1)!="H_OK")
    {
      return -1;
    }
  trID=sl.at(0).toUInt();
  lastBlock=sl.at(3).toUInt();
  fixBlockList.append(lastBlock++);
  inSeries=false;
  done=false;
  //  extended=false;
  for(i=4;(!done)&&i<sl.count();i++)
    {
      block=sl.at(i).toInt();
      if(block==-99)
        {
          done=true;
          i++;
          break;
        }
      if(block<0) inSeries=true;
      else
        {
          if(inSeries)
            {
              inSeries=false;
              for(j=lastBlock;j<block;j++) fixBlockList.append(j);
            }
          fixBlockList.append(block);
          lastBlock=block+1;
        }
    }
  // check if we have a filename beyond -99
  if((i+1)<sl.count()) // we need an additional 2 entries (filename and mode)
    {
      //      extended=true;
      //      fileName=sl.at(i++); // not used at this moment
    }
  return trID;
}


void drmTx::initDRMBSR(QByteArray *ba)
{
  baDRM=*ba;
  fixBlockList.clear();
  txDRM->init(&baDRM,"bsr","bin",drmTxParameters);
  addToLog(QString("bsr.bin send %1").arg(baDRM.size()),LOGPERFORM);
}

bool drmTx::initDRMFIX(txSession *sessionPtr)
{
  reedSolomonCoder rsd;
  eRSType rsType;
  rsType=(eRSType)sessionPtr->drmParams.reedSolomon;
  baDRM=sessionPtr->ba;
  if(rsType!=RSTNONE)
    {
      rsd.encode(baDRM,sessionPtr->extension,rsType);
      txDRM->init(&baDRM,sessionPtr->filename,rsTypeStr[rsType],sessionPtr->drmParams);
    }
  else
    {
      txDRM->init(&baDRM,sessionPtr->filename,sessionPtr->extension,sessionPtr->drmParams);
    }
  return true;
}


void drmTx::sendBSR(QByteArray *p,drmTxParams dp)
{
  setTxParams(dp);
  initDRMBSR(p);
  dispatcherPtr->startTX(txFunctions::TXSENDDRMBSR);
  addToLog("sendDRMBSR",LOGDRMTX);
}

txSession *drmTx::getSessionPtr(uint transportID)
{
  int i;
  for(i=0;i<txList.count();i++)
    {
      if(txList.at(i).transportID==transportID)
        {
          return &txList[i];
        }
    }
  return NULL;
}

void drmTx::applyTemplate(QString templateFilename, bool useTemplate, imageViewer *ivPtr)
{
  clearLastHybridUpload();
  forgetTxFileName();
  ivPtr->setParam(templateFilename,useTemplate);
}
