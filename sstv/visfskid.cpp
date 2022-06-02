#include "visfskid.h"
#include <QDebug>


const QString fskStateStr[fskIdDecoder::GETID+1]=
{
  "FSKINIT",
  "WAITS1500",
  "WAITE1500",
  "WAITS1900",
  "WAITE1900",
  "WAITS2100",
  "WAITE2100",
  "GETID"
};

const QString visStateStr[visDecoder::GETCODE+1]=
{
  "VISINIT",
  "WAITS1200",
  "WAITE1200",
  "WAITS1900",
  "WAITE1900",
  "GETCODE"
};

//const QString retraceStateStr[retraceDetector::RETRACEWAITEND+1]=
//{
//  "RETRACEINIT",
//  "RETRACESTART",
//  "RETRACEEND",
//};



fskDecoder::fskDecoder()
{
  sampleCounter=0;
  syncSampleCounter=0;
}


bool fskDecoder::waitStartFreq(unsigned int freqL,unsigned int freqH)
{
  if((avgFreq>=freqL) && (avgFreq<=freqH))
    {
      if(avgCounter<avgCount) avgCounter++;
      else
        {
          timeoutCounter=0;
          return true;
        }
    }
  else
    {
      if(avgCounter) avgCounter--;
    }
  return false;
}

bool fskDecoder::waitStartFreq(unsigned int freqL, unsigned int freqH, unsigned long maxWait, bool &timeout)
{
  if((avgFreq>=freqL) && (avgFreq<=freqH))
    {
      if(avgCounter<avgCount)
      {
          avgCounter++;
      }
      else
        {
          timeoutCounter=0;
          return true;
        }
    }
  else
    {
      if(avgCounter) avgCounter--;
    }
  if (timeoutCounter++>maxWait)
    {
      timeout=true;
    }
  else
    {
      timeout=false;
    }
  return false;
}

bool fskDecoder::waitEndFreq(unsigned int freqL,unsigned int freqH)
{
  if((avgFreq<freqL) || (avgFreq>freqH))
    {
      if(avgCounter) avgCounter--;
      else return true;
    }
  else
    {
      if(avgCounter<avgCount) avgCounter++;
    }
  return false;
}

bool fskDecoder::waitEndFreq(unsigned int freqL, unsigned int freqH, unsigned long maxWait, bool &timeout)
{
  if((avgFreq<freqL) || (avgFreq>freqH))
    {
      if(avgCounter<avgCount) avgCounter++;
      else
        {
          timeoutCounter=0;
          return true;
        }
    }
  else
    {
      if(avgCounter) avgCounter--;
    }
  if (timeoutCounter++>maxWait)
    {
      timeout=true;
    }
  else
    {
      timeout=false;
    }
  return false;
}




void fskDecoder::init()
{
  avgCounter=0;
  bitCounter=0;
  code=0;
  checksum=0;
  sampleCounter=0;
  timeoutCounter=0;
}



fskIdDecoder::fskIdDecoder()
{
  fskState=FSKINIT;
}




bool fskIdDecoder::assemble(bool reset)
{
  if(reset)
    {
      checksum=0;
      headerFound=false;
      endFound=false;
      fskIDStr.clear();
      return true;
    }
  if(!headerFound) // waiting for start
    {
      if(symbol==0x2A)
        {
          addToLog("headerFound",LOGFSKID);
          headerFound=true;
          return false;
        }
      return true; // indicate resync

    }
  if(symbol==0x01)
    {
      endFound=true;
      return false;
    }
  if(endFound)
    {
      if(checksum!=symbol)
        {
          fskIDStr.clear();
        }
      emit callReceived(fskIDStr);
      fskIDStr.clear();
      return true;
    }
  else
    {
      checksum=checksum^symbol;
      fskIDStr.append(symbol+0x20);
      addToLog(QString("fskstr %1 %2").arg(fskIDStr).arg(QString::number(symbol,16)),LOGFSKID);
      if(fskIDStr.length()>9)
        {
          fskIDStr.clear();
          return true;
        }
    }
  return false;
}

void fskIdDecoder::reset()
{
  avgCount=FSKAVGCOUNT;
  switchState(FSKINIT,0);
  init();
}

QString fskIdDecoder::getFSKId()
{
  QString tmp(fskIDStr);
  fskIDStr.clear();
  return  tmp;
}

void fskIdDecoder::switchState(efskState newState,unsigned int i)
{
  Q_UNUSED(i);
  addToLog(QString("%1 to %2 at samplecounter:%3 total: %4")
           .arg(fskStateStr[fskState])
           .arg(fskStateStr[newState])
           .arg(sampleCounter+i)
           .arg(syncSampleCounter+i)
           ,LOGFSKID);
  fskState=newState;
}


void fskIdDecoder::extract(unsigned int syncSampleCtr, bool narrow)
{
  int i;
  if(narrow) return;
  syncSampleCounter=syncSampleCtr;
  bool timeout;
  for(i=0;i<RXSTRIPE;i++)
    {
      avgFreq=freqPtr[i];
      switch (fskState)
        {
        case FSKINIT:
          sampleCounter=0;
          avgCounter=0;
          switchState(WAITSTART1500,i);
          timeoutCounter=0;
          break;
        case WAITSTART1500:
          if(waitStartFreq(1400,1600))
            {
              switchState(WAITEND1500,i);
              startSampleCounter=sampleCounter+i;
            }
          break;
        case WAITEND1500:
          {
            if(waitEndFreq(1400,1600))
              {
                if(((sampleCounter+i-startSampleCounter)>=FSKMIN1500))
                  {
                    switchState(WAITSTART2100,i);
                    timeoutCounter=0;
                  }
                else switchState(FSKINIT,i);
              }
          }
          break;

        case WAITSTART2100:
          if(waitStartFreq(2000,2200,100,timeout))
            {
              switchState(WAITEND2100,i);
              startSampleCounter=sampleCounter+i;
            }
          else if(timeout)
            {
              switchState(FSKINIT,i);
            }
          break;


        case WAITEND2100:
          {
            if(waitEndFreq(2000,2200))
              {
                if(((sampleCounter+i-startSampleCounter)>=FSKMIN2100) && ((sampleCounter+i-startSampleCounter)<2*FSKMIN2100))
                  {
                    switchState(WAITSTART1900,i);
                    timeoutCounter=0;
                  }
                else switchState(FSKINIT,i);
              }
          }
          break;

        case WAITSTART1900:
          if(waitStartFreq(1800,2000,50,timeout))
            {
              switchState(WAITEND1900,i);
              startSampleCounter=sampleCounter+i;
            }
          else if(timeout)
            {
              switchState(FSKINIT,i);
            }
          break;
        case WAITEND1900:
          {
            if((sampleCounter+i-startSampleCounter)>=(FSKBIT/2-avgCount-56))
              {
                switchState(GETID,i);
                startSampleCounter=sampleCounter+i;
                assemble(true);
                bitCounter=0;
                symbol=0;
              }
          }
          break;
        case GETID:
          if((sampleCounter+i-startSampleCounter)>=FSKBIT)
            {

              startSampleCounter=sampleCounter+i;
              symbol>>=1;

              if(avgFreq<2000)
                {
                  symbol|=0x20;
                }
              addToLog(QString("bit %1, %2 - %3 ").arg(bitCounter).arg(sampleCounter+i).arg(QString::number(symbol,2)),LOGFSKID);
              bitCounter++;
              if(bitCounter==6)
                {
                  if(assemble(false))
                    {
                      switchState(FSKINIT,i);
                    }
                  bitCounter=0;
                }
            }
          break;
        }
    }
  sampleCounter+=RXSTRIPE;
}



// VISDECODER
// h1 1900 Hz 300
// p1 1200 Hz 10
// h2 1900 Hz 300
// c1 1200 Hz 30
// d0 1100 Hz 30 (1)  - 1300 Hz (0)
// d1 1100 Hz 30 (1)  - 1300 Hz (0)
// d2 1100 Hz 30 (1)  - 1300 Hz (0)
// d3 1100 Hz 30 (1)  - 1300 Hz (0)
// d4 1100 Hz 30 (1)  - 1300 Hz (0)
// d5 1100 Hz 30 (1)  - 1300 Hz (0)
// d6 1100 Hz 30 (1)  - 1300 Hz (0)
// d7 1100 Hz 30 (1)  - 1300 Hz (0)
// c2 1200 Hz 30
visDecoder::visDecoder()
{
  visState=VISINIT;
}


void visDecoder::reset()
{
  avgCount=VISAVGCOUNT;
  switchState(VISINIT,0);
  init();
}

uint visDecoder::getCode()
{
  return code;
}

void visDecoder::switchState(evisState newState,unsigned int i)
{
  Q_UNUSED(i);
//  addToLog(QString("%1 to %2 at samplecounter:%3 samplecounter%4").arg(visStateStr[visState]).arg(visStateStr[newState]).arg(sampleCounter+i).arg(sampleCounter+i),LOGVISCODE);
  visState=newState;
}


void visDecoder::extract(unsigned int syncSampleCtr, bool narrow)
{
  syncSampleCounter=syncSampleCtr;
  if (narrow) extractNarrow();
  else extractWide();
}


void visDecoder::extractNarrow()
{

  int i;
  unsigned int syncTemp;
  Q_UNUSED(syncTemp);
  bool timeout=false;
  for(i=0;i<RXSTRIPE;i++)
    {
      syncTemp=syncSampleCounter+i;
      avgFreq=freqPtr[i];
      switch (visState)
        {
        case VISINIT:
          sampleCounter=0;
          avgCounter=0;
          switchState(WAITSTART1900,i);
          break;

        case WAITSTART1900:
          if(waitStartFreq(1800,2000))
            {
              switchState(WAITEND1900,i);
              startSampleCounter=sampleCounter+i;
            }
          break;
        case WAITEND1900:
          {
            if(waitEndFreq(1800,2000))
              {

                if(((sampleCounter+i-startSampleCounter)>=VISMIN1900) && ((sampleCounter+i-startSampleCounter)<2*VISMIN1900))
                  {
                    switchState(WAITSTART2100,i);
                  }
                else switchState(VISINIT,i);
              }
          }
          break;
        case WAITSTART2100:
          if(waitStartFreq(2000,2200,50,timeout))
            {
              switchState(WAITEND2100,i);
              startSampleCounter=sampleCounter+i;
            }
          else if(timeout)
            {
              switchState(VISINIT,i);
            }
          break;
         case WAITEND2100:
          if(waitEndFreq(2000,2200,1250,timeout))
            {
              switchState(WAITSTARTBIT,i);
              startSampleCounter=sampleCounter+i;
            }
           else if(timeout)
             {
                switchState(VISINIT,i);
             }
            break;
        case WAITSTARTBIT:
          if(waitEndFreq(1800,2000,VISBITNARROW/2,timeout))
              {
                switchState(VISINIT,i);  // too short
              }
            if (timeout)
              {
                validCode=false;
                switchState(GETCODE,i);
                startSampleCounter=sampleCounter+i;
                bitCounter=0;
                bit=1;
                symbol=0;
              }
               break;
        case GETCODE:
          if((sampleCounter+i-startSampleCounter)>=VISBITNARROW)
            {
              startSampleCounter=sampleCounter+i;
              if(avgFreq<2000)
                {
                  symbol|=bit; //24 bits
                }
              addToLog(QString("bit %1, %2 - %3 ").arg(bitCounter).arg(syncSampleCounter+i).arg(QString::number(symbol,2)),LOGFSKID);
              bitCounter++;
              bit<<=1;
              if(bitCounter==24)
                {
                  // check for validity
                  if((symbol&0xFFF)!=0x56D)
                    {
                      validCode=false;
                    }
                  else
                    {
                      quint8 NVIS =((symbol>>12)&0x3F);
                      quint8 XNVIS=((symbol>>18)&0x3F);
                      if((NVIS^0x15)!=XNVIS)
                        {
                          validCode=false;
                        }
                      else
                        {
                          validCode=true;
                          if((mode=lookupVIS(symbol))!=NOTVALID)
                            {
                              emit visCodeNarrowDetected((int)mode,syncSampleCounter+i);
                            }
                        }
                     switchState(VISINIT,i);
                    }
                }
              addToLog(QString("bit %1, %2 - %3 ").arg(bitCounter).arg(syncSampleCounter+i).arg(QString::number(symbol,2)),LOGFSKID);
            }
          break;
        default:
          switchState(VISINIT,i);
          break;
        }
    }
  sampleCounter+=RXSTRIPE;
}




void visDecoder::extractWide()
{
  int i;
  bool timeout=false;
  for(i=0;i<RXSTRIPE;i++)
    {
      avgFreq=freqPtr[i];
      switch (visState)
        {
        case VISINIT:
          sampleCounter=0;
          avgCounter=0;
          switchState(WAITSTART1900,i);
          break;

        case WAITSTART1900:
          if(waitStartFreq(1800,2000))
            {
              switchState(WAITEND1900,i);
              startSampleCounter=sampleCounter+i;
            }
          break;
        case WAITEND1900:
          {
            if(waitEndFreq(1800,2000))
              {

                if(((sampleCounter+i-startSampleCounter)>=VISMIN1900) && ((sampleCounter+i-startSampleCounter)<2*VISMIN1900))
                  {
                    addToLog(QString("end1900 at %1").arg(syncSampleCounter+i),LOGVISCODE);
                    switchState(WAITSTART1200,i);
                    timeoutCounter=0;
                  }
                else switchState(VISINIT,i);
              }
          }
          break;
        case WAITSTART1200:
          if(waitStartFreq(1100,1300,50,timeout))
            {
              addToLog(QString("start1200 at %1").arg(syncSampleCounter+i),LOGVISCODE);
              switchState(WAITEND1200,i);
              startSampleCounter=sampleCounter+i;
              timeoutCounter=0;
            }
          else if(timeout)
            {
              switchState(VISINIT,i);
            }
          break;
        case WAITEND1200:
          {
            if(waitEndFreq(1100,1300,VISBITWIDE/2-50,timeout))
              {
                switchState(VISINIT,i);  // too short
              }
            if (timeout)
              {
                validCode=false;
                switchState(GETCODE,i);
                startSampleCounter=sampleCounter+i;
                addToLog(QString("startbit at %1").arg(syncSampleCounter+i),LOGVISCODE);
                bitCounter=0;
                bit=1;
                symbol=0;
              }
          }
          break;
        case GETCODE:
          if((sampleCounter+i-startSampleCounter)>=VISBITWIDE)
            {
              if(avgFreq>1400)
                {
                  //end of VIS detected
                  validCode=true;
                  addToLog(QString("end bits at %1").arg(syncSampleCounter+i-VISBITWIDE/2),LOGVISCODE);
                  if (bitCounter<=11)
                  {
                    symbol&=0xFF;
                  }
                  else
                    {
                      symbol&=0xFFFF;
                    }
                  // check for validity
                  if((mode=lookupVIS(symbol))!=NOTVALID)
                    {
//                      emit visCodeWideDetected((int)mode,syncSampleCounter+i-VISBITWIDE/2);
                      emit visCodeWideDetected((int)mode,syncSampleCounter+i);
                    }

                  switchState(VISINIT,i);
                }
              else
                {
                  startSampleCounter=sampleCounter+i;
                  //              symbol>>=1;

                  if(avgFreq<1200)
                    {
                      symbol|=bit; //16 bits
                    }
                  addToLog(QString("bit %1, %2 - %3 ").arg(bitCounter).arg(syncSampleCounter+i).arg(QString::number(symbol,2)),LOGVISCODE);
                  bitCounter++;
                  bit<<=1;
                }
            }
          break;
        default:
          switchState(VISINIT,i);
          break;
        }
    }
  sampleCounter+=RXSTRIPE;
}


streamDecoder::streamDecoder(bool narrow)
{
  fskCoder.setDataPtr(avgBuffer);
  visCoder.setDataPtr(avgBuffer);
//  retracer.setDataPtr(avgBuffer);
  isNarrow=narrow;
}


void streamDecoder::reset()
{
  avgFreq=0;
  fskCoder.reset();
  visCoder.reset();
//  retracer.reset();
}

void streamDecoder::process(quint16 *freqPtr , unsigned int syncSampleCtr)
{

  int i;
  for(i=0;i<RXSTRIPE;i++)
    {
      avgFreq=avgFreq*(1-FREQAVG)+FREQAVG*(DSPFLOAT)freqPtr[i];
      avgBuffer[i]=avgFreq;
    }

    fskCoder.extract(syncSampleCtr,isNarrow);
    visCoder.extract(syncSampleCtr,isNarrow);
  //  retracer.extract();
}








