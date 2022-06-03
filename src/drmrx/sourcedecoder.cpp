/**************************************************************************
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

#include "sourcedecoder.h"
#include "drm.h"
#include "appglobal.h"
#include "drmproto.h"
#include "configparams.h"
#include "dispatch/dispatcher.h"
#include "reedsolomoncoder.h"
#include "demodulator.h"
#include "ftpfunctions.h"
#include "configparams.h"
#include "logbook/logbook.h"
#include "drmstatusframe.h"


#include <QThread>
#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <math.h>
#include <float.h>

sourceDecoder::sourceDecoder(QObject *parent) : QObject(parent)
{
  transportBlockPtrList.clear();
  ff=new ftpFunctions;
  connect(ff,SIGNAL(downloadDone(bool,QString)),SLOT(slotDownloadDone(bool,QString)));

}

void sourceDecoder::init()
{
  lastTransportBlockPtr=NULL;
  bodyTotalSegments=0;
  checkIt=false;
  erasureList.clear();
  lastContinuityIndex=-1;
  alreadyDisplayed=false;
}


/*!
 \brief decode of valid data block

• header     8 bits.
• data field n bytes.
• CRC        16 bits.


 \return bool return true if successful
*/
bool sourceDecoder::decode()
{
  double checksum;
  int N_partB;
  //  if(!demodulatorPtr->isTimeSync())

  if (channel_decoded_data_buffer_data_valid != 1)  return false;
  if (audio_data_flag == 0)
    {
      addToLog("audio decoding not implemented in qsstv !\n",LOGDRMSRC); return false;
    }
  addToLog("Datapacket received",LOGPERFORM);
  N_partB = (int) (length_decoded_data/ 8);
  addToLog(QString("N-partB lenght=%1").arg(N_partB),LOGDRMSRC);
  if(N_partB>PACKETBUFFERLEN)
    {
      addToLog(QString("packet buffer length exceeded: lenght=%1").arg(N_partB),LOGDRMMOT);
    }
  bits2bytes (channel_decoded_data_buffer, N_partB * 8, packetBuffer);
  crc16_bytewise(&checksum, packetBuffer,N_partB);
  if(fabs (checksum) <= DBL_EPSILON)
    {
      if(!setupDataBlock(packetBuffer,true,N_partB))
        {
          msc_valid=INVALID;
          return false;
        }
    }
  else
    {
      msc_valid=INVALID;
      return false;
    }
  // at this point we have a dataPacket we now check header / data and buils a transport stream
  switch(currentDataPacket.dataGroupType)
    {
    case MOTDATA:  addToLog("Datasegment",LOGDRMSRC);   addDataSegment(); break;
    case MOTHEAD:  addToLog("Headersegment",LOGDRMSRC); addHeaderSegment(); break;
    default:
      return false;
      break;
    }
  return true;
}



bool sourceDecoder::setupDataBlock(unsigned char *buffer,bool crcIsOK,int len)
{
  currentDataBlock.length=len;
  unsigned char header=buffer[0];
  unsigned int availableBytes;
  const char *bufPtr;
  currentDataBlock.ba=QByteArray((char *)buffer,len);
  currentDataBlock.firstFlag=currentDataBlock.lastFlag=false;
  if(header & 0x80) currentDataBlock.firstFlag = true;
  if(header & 0x40) currentDataBlock.lastFlag  = true;
  currentDataBlock.packetID = (header & 0x30) >> 4;
  currentDataBlock.PPI = (header & 0x8) >> 3;
  currentDataBlock.continuityIndex = (header & 0x7);
  currentDataBlock.log();
  if ((currentDataBlock.PPI != 0) && (crcIsOK))
    {
      availableBytes=buffer[1];
      bufPtr=(const char *)&buffer[2];
    }
  else
    {
      availableBytes=len-3;
      bufPtr=(const char *)&buffer[1];
    }
  if(currentDataBlock.firstFlag)
    {
      holdingBuffer.clear();
      lastContinuityIndex=currentDataBlock.continuityIndex;
    }
  else
    {
      if(lastContinuityIndex<0)
        {
          return false;
        }
      lastContinuityIndex=(lastContinuityIndex+1)%8;
      if(currentDataBlock.continuityIndex!=lastContinuityIndex)
        {
          lastContinuityIndex=-1;
          return false;
        }

    }
  holdingBuffer.append(bufPtr,availableBytes);
  if(currentDataBlock.lastFlag)
    {
      return setupDataPacket(holdingBuffer);
    }
  return false;
}

void dataBlock::log()
{
  addToLog(QString("FFlag %1,LFlag %2, PacketID %3,PPI %4,ContIdx %5, CRC %6 len %7")
           .arg(firstFlag).arg(lastFlag).arg(packetID).arg(PPI).arg(continuityIndex).arg(crcOK).arg(length),LOGDRMMOT);
}

//MSC Header Description 2 bytes or 4 bytes if Extension field
// First Byte
// B7 Extension Flag    Header has 2 more bytes in the Extension field
// B6 CRC Flag          MOT has CRC
// B5 Session Flag      MOT last flag and segment number present.
// B4 UserAccess Flag   Mot user access field present.
// B3-B0 Data Group Type
//    0 0 0 0  (0) General data;
//    0 0 0 1  (1) CA messages (for example ECMs or EMMs: see subclause 9.3.2.1);
//    0 0 1 0  (2) General data and CA parameters (for example, DGCA);
//    0 0 1 1  (3) MOT header information;
//    0 1 0 0  (4) MOT data
//    0 1 0 1  (5) MOT data and CA parameters.

//Second Byte
// B7-B4 Continuity Index
// B3-B0 Repetition Index

// 2 more extension bytes if flag is set
// B15-B0 Extension Field - no further specifications

// Session Header (presence indicated by Session Flag

// if last flag and segment number present
// B15    last segment
// B14-B0 segment number

// user access fields
// B7-B5  rfa (not used)
// B4     TransportID present flag
// B3-B0  length of user access fields

// if set
// B15-B0  TransportID
// other bytes filled with End user address field

// This is followed with a 2 bytes Segmentation header
// B15-B13 Repetition Count (not used)
// B12-B0  Length of the data that follows


bool sourceDecoder::setupDataPacket(QByteArray ba)
{
  double checksum;
  unsigned char lengthIndicator;
  unsigned char header;

  currentDataPacket.ba=ba; // drop crc
  header=ba.at(0);
  currentDataPacket.transportID =0xFFFF;
  currentDataPacket.extFlag=false;
  currentDataPacket.crcFlag=false;
  currentDataPacket.sessionFlag=false;
  currentDataPacket.userFlag=false;
  currentDataPacket.lastSegment=false;
  currentDataPacket.crcOK=currentDataBlock.crcOK;

  if(header&0x10) currentDataPacket.userFlag=true;
  currentDataPacket.dataGroupType=(edataGroupType)(header&0x07);
  if(header&0x80) currentDataPacket.extFlag=true;
  if(header&0x20) currentDataPacket.sessionFlag=true;

  if(header&0x40)
    {
      currentDataPacket.crcFlag=true;
      crc16_bytewise (&checksum,(unsigned char *)currentDataPacket.ba.data(),currentDataPacket.ba.count());
      if (fabs (checksum) <= DBL_EPSILON)
        {
          currentDataPacket.crcOK=true;
        }
      else
        {
          currentDataPacket.crcOK=false;
          msc_valid=INVALID;
          return false;
        }
      currentDataPacket.chop(2); // drop crc
    }
  currentDataPacket.advance(2); //skip header and continuity bits
  if(currentDataPacket.extFlag) currentDataPacket.advance(2); // just skip the extension bytes

  if(currentDataPacket.sessionFlag)
    {
      currentDataPacket.segmentNumber = (((unsigned char)(currentDataPacket.ba.at(0)) & 0x7F))*256+ ((unsigned char)currentDataPacket.ba.at(1))  ;
      if(currentDataPacket.ba.at(0)&0x80)
        {
          currentDataPacket.lastSegment=true;
        }
      currentSegmentNumber=currentDataPacket.segmentNumber;
      currentDataPacket.advance(2);
    }

  if (currentDataPacket.userFlag)
    {
      currentDataPacket.userAccessField = (unsigned char)(currentDataPacket.ba.at(0));
      currentDataPacket.advance(1);
      lengthIndicator = (currentDataPacket.userAccessField& 0xF);

      if((currentDataPacket.userAccessField & 0x10) && (lengthIndicator>=2)) currentDataPacket.transportID = (((unsigned char)(currentDataPacket.ba.at(0))))*256+ ((unsigned char)currentDataPacket.ba.at(1))  ;
      currentDataPacket.advance(lengthIndicator);
    }
  currentDataPacket.segmentSize=(((unsigned char)(currentDataPacket.ba.at(0)) & 0x1F))*256+ ((unsigned char)currentDataPacket.ba.at(1));
  currentDataPacket.advance(2);
  currentDataPacket.lenght=currentDataPacket.ba.count();
  currentDataPacket.log();
  return true;
}



void dataPacket::log()
{
  addToLog(QString("extF %1,dType %2, sessionF %3, lastSegment %4, segment# %5, userF %6, transportId %7, len %8")
           .arg(extFlag).arg(dataGroupType).arg(sessionFlag).arg(lastSegment).arg(segmentNumber).arg(userFlag).arg(transportID).arg(lenght),LOGDRMMOT);
}



bool sourceDecoder::addHeaderSegment()
{
  loadRXImageEvent *stce;
  displayMBoxEvent *stmb;
  transportBlock *tbPtr;
  addToLog(QString("Header segsize: %1").arg(currentDataPacket.segmentSize),LOGDRMSRC);
  tbPtr=getTransporPtr(currentDataPacket.transportID,true);
  if(!tbPtr->alreadyReceived) msc_valid=VALID;
  else
    {
      msc_valid=ALREADYRECEIVED;
      if(!alreadyDisplayed)
        {
          alreadyDisplayed=true;
          // redisplay
          stce= new loadRXImageEvent(QString("%1").arg(tbPtr->newFileName));
          QApplication::postEvent( dispatcherPtr, stce );  // Qt will delete it when done
          stmb= new displayMBoxEvent("DRM Receive",QString("File %1 already received").arg(tbPtr->newFileName));
          QApplication::postEvent( dispatcherPtr, stmb );
        }



      return true;
    }

  tbPtr->headerReceived=true;
  unsigned char *dataPtr=(unsigned char *)currentDataPacket.ba.data();
  unsigned char PLI;
  unsigned char paramID;
  unsigned char extBit;
  unsigned short dataFieldLength;
  tbPtr->bodySize =
      (((unsigned int)dataPtr[0]) << 20) +
      (((unsigned int)dataPtr[1]) << 12) +
      (((unsigned int)dataPtr[2]) << 4) +
      ((((unsigned int)dataPtr[3]) & 0xF0) >> 4);

  tbPtr->headerSize =
      (((unsigned int)dataPtr[3] & 0x0F) << 9) +
      (((unsigned int)dataPtr[4]) << 1) +
      ((((unsigned int)dataPtr[5]) & 0x80) >> 7);

  tbPtr->contentType = (((unsigned int)dataPtr[5] & 0x7E) >> 1);
  tbPtr->contentSubtype = ((((unsigned int)dataPtr[5]) & 0x1) <<8) +((unsigned int)dataPtr[6]);
  currentDataPacket.advance(7); // size of header core
  // The header core is followed by a number of parameter blocks
  // the first byte of every parameter block contains a 2-bits PLI (B7 and B6) indicating the type of parameter block.

  while(currentDataPacket.ba.count())
    {
      PLI=dataPtr[0]>>6;
      paramID=dataPtr[0]&0x3F;
      switch (PLI)
        {
        case 0:
          currentDataPacket.advance(1);
          break;
        case 1:
          loadParams(tbPtr,paramID,1);
          currentDataPacket.advance(2);
          break;
        case 2:
          loadParams(tbPtr,paramID,4);
          currentDataPacket.advance(5);
          break;
        case 3:
          extBit=dataPtr[0]&0x80;
          if(extBit)
            {
              dataFieldLength=256*(dataPtr[0]&0x7F)+dataPtr[1];
              currentDataPacket.advance(2);
            }
          else
            {
              dataFieldLength=dataPtr[0]&0x7F;
              currentDataPacket.advance(1);
            }
          loadParams(tbPtr,paramID,dataFieldLength);
          currentDataPacket.advance(dataFieldLength);
          break;
        }
    }
  return true;

}

void sourceDecoder::loadParams(transportBlock *tbPtr,unsigned char paramID,int len)
{
  //  QByteArray testBA;
  //  char *data;
  //  unsigned int ct;
  rxDRMStatusEvent *stce;
  QString tmp,t;
  switch(paramID)
    {
    case 5: // expiration
      break;
    case 6:
      break;
    case 12:
      tbPtr->fileName=QString::fromLatin1(currentDataPacket.ba.data()+1).left(len-1);
      stce= new rxDRMStatusEvent(QString("%1").arg(tbPtr->fileName));
      QApplication::postEvent( dispatcherPtr, stce );  // Qt will delete it when done
      break;
    default:
      break;
    }
}


void sourceDecoder::addDataSegment()
{
  int i;
  transportBlock *tbPtr;
  tbPtr=getTransporPtr(currentDataPacket.transportID,true);
  rxTransportID=currentDataPacket.transportID;
  if(callsignValid) tbPtr->callsign=drmCallsign;
  addToLog(QString("Data segsize: %1 segment# %2").arg(currentDataPacket.segmentSize).arg(currentDataPacket.segmentNumber),LOGDRMSRC);


  if(!tbPtr->alreadyReceived) msc_valid=VALID;
  else
    {
      msc_valid=ALREADYRECEIVED;
      //      return;
    }
  if(currentDataPacket.lastSegment)
    {
      tbPtr->totalSegments=currentDataPacket.segmentNumber+1;
      tbPtr->lastSegmentReceived=true;
    }
  else
    {
      tbPtr->defaultSegmentSize=currentDataPacket.segmentSize;
    }
  for(i=tbPtr->dataSegmentPtrList.count();i<=currentDataPacket.segmentNumber;i++)
    {
      tbPtr->dataSegmentPtrList.append(new dataSegment(tbPtr->defaultSegmentSize));
    }

  if(!tbPtr->dataSegmentPtrList.at(currentDataPacket.segmentNumber)->hasData())
    {
      checkIt=true;
    }
  else
    {
      msc_valid=ALREADYRECEIVED;
    }
  if(tbPtr->alreadyReceived)
    {
      msc_valid=ALREADYRECEIVED;
      checkIt=false;
    }
  if(tbPtr->totalSegments<currentDataPacket.segmentNumber+1) tbPtr->totalSegments=currentDataPacket.segmentNumber+1;
  bodyTotalSegments=tbPtr->totalSegments;
  rxSegments=tbPtr->segmentsReceived;
  //  bytesReceived=rxSegments*tbPtr->defaultSegmentSize;

  tbPtr->dataSegmentPtrList.at(currentDataPacket.segmentNumber)->setData(currentDataPacket.ba,currentDataPacket.segmentNumber,true);

  writeData(tbPtr);

}

void sourceDecoder::writeData(transportBlock *tbPtr)
{
  int i;

  QByteArray ba;

  int length=0;
  erasureList.clear();
  erasureList.append(tbPtr->totalSegments);
  erasureList.append(tbPtr->defaultSegmentSize);
  addToLog("Write data",LOGDRMSRC);
  for(i=0;i<tbPtr->dataSegmentPtrList.count();i++)
    {
      if(!tbPtr->dataSegmentPtrList.at(i)->hasData())
        {
          erasureList.append(i);
          ba.append(QByteArray(tbPtr->defaultSegmentSize,0x00));
        }
      else
        {
          ba.append(tbPtr->dataSegmentPtrList.at(i)->data);
        }
      length+=tbPtr->dataSegmentPtrList.at(i)->data.size();
    }
  tbPtr->segmentsReceived=0;
  drmBlockList.clear();
  for(i=0;i<tbPtr->dataSegmentPtrList.count();i++)
    {
      if(tbPtr->dataSegmentPtrList.at(i)->hasData())
        {
          drmBlockList.append(i);
          tbPtr->segmentsReceived++;
        }
    }
  if(tbPtr->isAlmostComplete()<63)
    {
      return ;
    }
  addToLog("Image already received",LOGDRMSRC);

  if(!tbPtr->lastSegmentReceived)
    {
      addToLog("Not lastSegmentReceived",LOGDRMSRC);

      return;
    }
  checkSaveImage(ba,tbPtr);
}


void sourceDecoder::saveImage(transportBlock *tbPtr)
{
  int i;
  QByteArray hybridBa;
  displayMBoxEvent *stmb=0;
  QString downloadF;
  modeCodeTmp=tbPtr->modeCode;
  callsignTmp=tbPtr->callsign;

  alreadyDisplayed=true;
  if(tbPtr->alreadyReceived)
    {
      addToLog("Image already received",LOGDRMSRC);
      return ;
    }
  if(tbPtr->fileName.isEmpty()) return ;
  if(tbPtr->retrieveTries==0) lastAvgSNR=avgSNR;
  isHybrid=false;
  if((tbPtr->fileName.left(3)==".de") || (tbPtr->fileName.left(3)=="de_"))
    {
      isHybrid=true;
      //ftpInterface ftpIntf("Save Image FTP");
      if((enableHybridRx) && (soundRoutingInput!=soundBase::SNDINFROMFILE))
        {
          addToLog(QString("Hybrid filename %1, attempt %2").arg(tbPtr->fileName).arg(tbPtr->retrieveTries+1),LOGALL);
          downloadF=rxDRMImagesPath+"/"+tbPtr->fileName;
          for(i=0;i<tbPtr->dataSegmentPtrList.count();i++)
            {
              hybridBa+=tbPtr->dataSegmentPtrList.at(i)->data;
            }
          if(hc.deCrypt(&hybridBa))
            {
              if (tbPtr->retrieveTries==0)
                {
                  ff->setupFtp("GetHybridImage",hc.host(),hc.port(),hc.user(),hc.passwd(),hc.dir()+"/"+hybridFtpHybridFilesDirectory);

                  ff->downloadFile(tbPtr->fileName.toLatin1(),downloadF,false,false);
                }
              tbPtr->retrieveTries++;

            }
          else
            {
              if (tbPtr->retrieveTries==0)
                {
                  stmb= new displayMBoxEvent("Hybrid Error","No file downloaded");
                  QApplication::postEvent( dispatcherPtr, stmb );  // Qt will delete it when done
                  return;
                }
            }
        }
      else
        {
          downloadF.clear();
        }
      tbPtr->newFileName=downloadF;
    }  // end of hybrid.
  else
    {
      displayReceivedImage(false,tbPtr->newFileName);
    }
  tbPtr->setAlreadyReceived(true);
}


void sourceDecoder::slotDownloadDone(bool err,QString filename)
{
  if(err) return;
  displayReceivedImage(true,filename);
  // send notification

  QFileInfo fi(filename);
  QString fn=fi.fileName();

  QString notificationFn = "Dummy"+fn+"+++."+myCallsign+QString("  %1dB SNR").arg(lastAvgSNR,0,'f',0);
  if (enableHybridNotify && !hybridNotifyDir.isEmpty())
    {
      ff->changeThreadName("Notify RX");
      ff->changePath("/"+hc.dir()+"/"+hybridNotifyDir,false);
      ff->mremove("*+++."+myCallsign+" *",false,false);
      ff->uploadData("Dummy\r\n",notificationFn,false,true);
    }
  else
    {
      ff->disconnect();
    }
}



void sourceDecoder::displayReceivedImage(bool isHybrid,QString filename)
{
  bool saveOK=false;
  bool done=false;
  bool textMode=false;
  QString t;
  displayTextEvent *stce;

  QImage test;
  if(filename.isEmpty()) return ;
  if(!test.load(filename))
    {
      // maybe text
      QFileInfo finfo(filename);
      if((finfo.suffix()=="txt") || (finfo.suffix()=="chat") )
        {
          QFile fi(filename);
          if(!fi.open(QIODevice::ReadOnly)) return;
          t=fi.readAll();
          stce= new displayTextEvent(t);
          QApplication::postEvent( dispatcherPtr, stce );  // Qt will delete it when done
          textMode=true;
        }
      saveOK=true;
    }
  else
    {
      saveOK=true;
    }
  if(saveOK)
    {
      QFileInfo tfi(filename);
      QString modestr(tfi.fileName());
      modestr+=QString(" %1dB ").arg(lastAvgSNR,0,'f',0);
      if(isHybrid) modestr+="Hybrid ";
      modestr+=compactModeToString(modeCodeTmp);
      logBookPtr->logQSO(callsignTmp,"DSSTV",modestr);
    }

  if(!textMode)
    {
      QString info="";
      if (isHybrid) info+="Hybrid";
      else info+=compactModeToString(modeCodeTmp);

      info+=QString(" %2dB de %3").arg(lastAvgSNR,0,'f',0).arg(callsignTmp);
      //    slotRXNotification("*** "+info);
      saveDRMImageEvent *ce = new saveDRMImageEvent(filename,info);
      ce->waitFor(&done);
      QApplication::postEvent(dispatcherPtr, ce);
      while(!done)
        {
          qApp->processEvents();
          usleep(10);
        }
      checkIt=false;
    }
}


bool sourceDecoder::checkSaveImage(QByteArray ba,transportBlock *tbPtr)
{
  prepareFixEvent *pe;
  QFile outFile;
  reedSolomonCoder rsd;
  QString fileName;
  QString extension;
  fileName=rxDRMImagesPath+"/"+tbPtr->fileName;
  tbPtr->newFileName=fileName;
  QByteArray baFile;
  QByteArray *baFilePtr;

  if(!checkIt) return false;
  QFileInfo qfinf(fileName);
  extension=qfinf.suffix().toLower();
  if((extension=="rs1") || (extension=="rs2") ||(extension=="rs3")||(extension=="rs4"))
    {
      // try to decode
      addToLog("Try to decode",LOGDRMSRC);
      if(tbPtr->alreadyReceived) return false;
      if(!rsd.decode(ba,fileName,tbPtr->newFileName,baFile,extension,erasureList)) return false;
      baFilePtr=&baFile;
    }
  else
    {
      addToLog("Check complete",LOGDRMSRC);
      if(!tbPtr->isComplete()) return false;
      tbPtr->newFileName=fileName;
      if((tbPtr->fileName=="bsr.bin")&&(!tbPtr->alreadyReceived))
        {
          tbPtr->setAlreadyReceived(true);
          pe = new prepareFixEvent(ba);
          QApplication::postEvent(dispatcherPtr, pe);
          return false;
        }
      baFilePtr=&ba;
    }
  if(!tbPtr->alreadyReceived)
    {
      outFile.setFileName(tbPtr->newFileName);
      if(outFile.open(QIODevice::WriteOnly)<=0)
        {
          outFile.close();
          return false;
        }
      outFile.write(*baFilePtr);
      outFile.close();
      erasureList.clear();
      saveImage(tbPtr);
    }
  return false;
}

QList <bsrBlock> *sourceDecoder::getBSR()
{
  int i;
  transportBlock *tbPtr;
  bsrList.clear();

  for(i=0;i<transportBlockPtrList.count();i++)
    {
      tbPtr=transportBlockPtrList.at(i);
      if(tbPtr->alreadyReceived) continue;
      if(tbPtr->fileName=="bsr.bin") continue;
      bsrList.append(bsrBlock(tbPtr));
    }
  return &bsrList;
}

bool sourceDecoder::storeBSR(transportBlock *tb, bool compat)
{
  int i;
  int needsFiller;
  int prevErasure=0;
  // QByteArray ba;
  erasureList.clear();
  erasureList.append(tb->totalSegments);
  erasureList.append(tb->defaultSegmentSize);
  for(i=0;i<tb->dataSegmentPtrList.count();i++)
    {
      if(!tb->dataSegmentPtrList.at(i)->hasData())
        {
          erasureList.append(i);
        }
    }
  tb->baBSR.clear();
  if(erasureList.count()<3) return false; //erasurelist has already totalSegments and defaultSegmentSize
  tb->baBSR.append(QString::number(tb->transportID).toLatin1().data());
  tb->baBSR.append("\n");
  tb->baBSR.append("H_OK\n");
  tb->baBSR.append(QString::number(erasureList.at(1)).toLatin1().data());
  tb->baBSR.append("\n");
  tb->baBSR.append(QString::number(erasureList.at(2)).toLatin1().data());
  tb->baBSR.append("\n");

  prevErasure=erasureList.at(2);
  needsFiller=false;
  for(i=3;i<erasureList.count();i++) //skip
    {
      if(((prevErasure+1)==erasureList.at(i))&&(compat))
        {
          needsFiller=true;
        }
      else
        {
          if(needsFiller)
            {
              tb->baBSR.append(QString::number(-1).toLatin1().data());
              tb->baBSR.append("\n");
              needsFiller=false;
            }
          tb->baBSR.append(QString::number(erasureList.at(i)).toLatin1().data());
          tb->baBSR.append("\n");
        }
      prevErasure=erasureList.at(i);
    }
  if(needsFiller)
    {
      tb->baBSR.append(QString::number(-1).toLatin1().data());
      tb->baBSR.append("\n");
      needsFiller=false;
      tb->baBSR.append(QString::number(erasureList.at(erasureList.count()-1)).toLatin1().data());
      tb->baBSR.append("\n");
    }
  tb->baBSR.append("-99\n");
  if(!compat)
    {
      QString temp;
      tb->baBSR.append(tb->fileName.toLatin1()+"\n");
      temp=QString::number(tb->modeCode);
      while(temp.length()<5) temp.prepend("0");
      tb->baBSR.append(temp.toLatin1());
    }
  return true;
}


transportBlock *sourceDecoder::getTransporPtr(unsigned short tId,bool create)
{
  int i;
  rxDRMStatusEvent *stce;
  bool found=false;
  for(i=0;i<transportBlockPtrList.count();i++)
    {
      if(transportBlockPtrList.at(i)->transportID==tId)
        {
          found=true;
          break;
        }
    }
  if(found) lastTransportBlockPtr=transportBlockPtrList.at(i);
  else if (create)
    {
      callsignValid=false;
      bodyTotalSegments=0;
      drmBlockList.clear();
      stce= new rxDRMStatusEvent("");
      QApplication::postEvent( dispatcherPtr, stce );  // Qt will delete it when done
      if(transportBlockPtrList.count()>=MAXTRANSPORTLISTS)
        {
          //delete the oldest
          transportBlockPtrList.removeFirst();
        }
      for(i=0;i<transportBlockPtrList.count();)
        {
          if(transportBlockPtrList.at(i)->fileName=="bsr.bin")
            {
              transportBlockPtrList.takeAt(i);
            }
          else i++;
        }
      transportBlockPtrList.append(new transportBlock(tId));
      lastTransportBlockPtr=transportBlockPtrList.last();
      lastTransportBlockPtr->robMode=robustness_mode;
      lastTransportBlockPtr->interLeaver=interleaver_depth_new;
      lastTransportBlockPtr->mscMode=msc_mode_new; // qam
      lastTransportBlockPtr->mpx=multiplex_description.PL_PartB;
      lastTransportBlockPtr->spectrum=spectrum_occupancy_new;
      // remap msc_new to modeCode
      int mCode=1;    //default QAM16
      if(msc_mode_new==3) mCode=0;
      if(msc_mode_new==0) mCode=2;
      int protection=0;
      if(multiplex_description.PL_PartB==1) protection=1;
      lastTransportBlockPtr->modeCode=robustness_mode*10000+spectrum_occupancy_new*1000+protection*100+mCode*10+interleaver_depth_new;
    }
  else
    {
      return NULL;
    }
  return lastTransportBlockPtr;
}

void sourceDecoder::removeTransporPtr(transportBlock * ptr)
{
  int i;
  for(i=0;i<transportBlockPtrList.count();i++)
    {
      if(transportBlockPtrList.at(i)==ptr)
        {
          transportBlockPtrList.takeAt(i);
          break;
        }
    }
}
