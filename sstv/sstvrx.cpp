#include "sstvrx.h"
#include "appglobal.h"
#include "configparams.h"
#include "dispatcher.h"
#include "filterparam.h"
#include "filters.h"
#include "modes/modebase.h"


#ifndef QT_NO_DEBUG
#include "arraydumper.h"
#include "scope/scopeview.h"
#include "scope/scopeoffset.h"
#endif

QString defaultImageFormat;
int minCompletion;

#define DECAY 0.9956

#ifdef DISABLENARROW
  #pragma message "NARROW DETECTION DISABLED"
#endif

const QString stateStr[sstvRx::END+1]=
{
  "Hunting",
  "Processing",
  "WaitForSync",
  "SlantAdjust",
  "Restart",
  "Sync Lost",
  "Wait",
  "End"
};


sstvRx::sstvRx(QObject *parent) : QObject(parent),syncNarrowProc(true),syncWideProc(false)
{
  syncFilterPtr=NULL;
  videoFilterPtr=NULL;
  syncProcPtr=NULL;
#ifndef QT_NO_DEBUG
  scopeViewerData=new scopeView("Data Scope");
  scopeViewerSyncNarrow=new scopeView("Sync Scope Narrow");
  scopeViewerSyncWide=new scopeView("Sync Scope Wide");
#endif
}

void sstvRx::init()
{
  setFilters(); // setup sstvRx Filters
  resetParams(true);
#ifndef QT_NO_DEBUG
  scopeViewerData->setAlternativeScaleMultiplier(SUBSAMPLINGFACTOR/rxClock);
  scopeViewerData->setCurveName("RX VOL",SCDATA1);
  scopeViewerData->setCurveName("TEST",SCDATA2);
  scopeViewerData->setCurveName("STATE",SCDATA3);
  scopeViewerData->setCurveName("DEMOD",SCDATA4);
  scopeViewerData->setAxisTitles("Samples","int","State");
  scopeViewerSyncNarrow->setAlternativeScaleMultiplier(SUBSAMPLINGFACTOR/rxClock);
  scopeViewerSyncWide->setAlternativeScaleMultiplier(SUBSAMPLINGFACTOR/rxClock);
#endif
}
void sstvRx::resetParams(bool bufferReset)
{
  SSTVState=HUNTING;
  bufferCounter=0;
  if(bufferReset)
    {
      addToLog("reset Buffers",LOGRXMAIN);
      agcVolume=0;
      bufferVideoDemod.reset();
      bufferSync1200Vol.reset();
      bufferSync1900Vol.reset();
      bufferInputVol.reset();
      syncNarrowProc.reset();
      syncWideProc.reset();
    }
  else
    {
      syncNarrowProc.init();
      syncWideProc.init();
    }
}

sstvRx::~sstvRx()
{
  if(videoFilterPtr!=NULL) delete videoFilterPtr;
  if(syncFilterPtr !=NULL) delete syncFilterPtr;
}

void sstvRx::setFilters()
{
  if(videoFilterPtr==NULL)
    videoFilterPtr=new videoFilter(RXSTRIPE);
  else
    videoFilterPtr->init();
  if(syncFilterPtr==NULL) syncFilterPtr=new syncFilter(RXSTRIPE);
  else syncFilterPtr->init();
}


//void sstvRx::run(DSPFLOAT *dataPtr, unsigned int *inputVolumePtr)
void sstvRx::run(DSPFLOAT *dataPtr,DSPFLOAT *volumePtr)
{
  bufferCounter++;
  if(videoFilterPtr==NULL)
    {
      errorOut() << "videoFltr NULL";
      return;
    }
  // run AGC
//  for(i=0;i<RXSTRIPE;i++)
//    {
//      DSPFLOAT tmp=sqrt(dataPtr[i]*dataPtr[i]);
//      if(tmp>agcVolume) agcVolume=tmp;
//      else agcVolume*=DECAY;
//      if(agcVolume<1000) agcVolume=1000;  //maximum gain is 8
//      dataPtr[i]=dataPtr[i]*8000/agcVolume;
//    }
  videoFilterPtr->process(dataPtr);
  bufferVideoDemod.putNoCheck(videoFilterPtr->demodPtr,RXSTRIPE);
  syncFilterPtr->process(dataPtr);
  bufferSync1200Vol.putNoCheck(syncFilterPtr->detect1200Ptr,RXSTRIPE);
#ifndef DISABLENARROW
  bufferSync1900Vol.putNoCheck(syncFilterPtr->detect1900Ptr,RXSTRIPE);
#endif
  bufferInputVol.putNoCheck(volumePtr ,RXSTRIPE);

  while (bufferVideoDemod.count())
    {
      syncWideProc.freqPtr=bufferVideoDemod.readPointer();
      syncNarrowProc.freqPtr=bufferVideoDemod.readPointer();
      syncWideProc.syncVolumePtr =bufferSync1200Vol.readPointer();
      syncNarrowProc.syncVolumePtr =bufferSync1900Vol.readPointer();
      syncWideProc.inputVolumePtr=bufferInputVol.readPointer();
      syncNarrowProc.inputVolumePtr=bufferInputVol.readPointer();

      displaySyncEvent* ce;
      if(syncProcPtr)
        {
          ce = new displaySyncEvent(syncProcPtr->syncQuality);
        }
      else
        {
          ce = new displaySyncEvent(0);
        }
      QApplication::postEvent(dispatcherPtr, ce);
#ifndef QT_NO_DEBUG
      scopeViewerData->addData(SCDATA1,bufferInputVol.readPointer(),syncWideProc.sampleCounter,RXSTRIPE);
      scopeViewerData->addData(SCDATA2,dataPtr,syncWideProc.sampleCounter,RXSTRIPE);
      scopeViewerData->addData(SCDATA4,bufferVideoDemod.readPointer(),syncWideProc.sampleCounter,RXSTRIPE);
#endif

      process();
    }
  //  addToLog(QString("After process readIndex:=%1 sampleCounter:=%2").arg(bufferVideoDemod.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
}

void sstvRx::advanceBuffers()
{
  syncWideProc.sampleCounter+=RXSTRIPE;
  syncNarrowProc.sampleCounter+=RXSTRIPE;
  bufferVideoDemod.skip(RXSTRIPE);
  bufferSync1200Vol.skip(RXSTRIPE);
  bufferSync1900Vol.skip(RXSTRIPE);
  bufferInputVol.skip(RXSTRIPE);
}

void sstvRx::rewindBuffers(uint rlen)
{
  syncWideProc.sampleCounter-=rlen;
  syncNarrowProc.sampleCounter-=rlen;
  bufferVideoDemod.rewind(rlen);
  bufferSync1200Vol.rewind(rlen);
  bufferSync1900Vol.rewind(rlen);
  bufferInputVol.rewind(rlen);
}




void sstvRx::process()
{
//  unsigned int ri;
  quint32 block;
  quint32 syncPosition;
  quint32 sampleCounterLatch;
  quint32 diff;

  syncProcessor::esyncProcessState syncStateWide;
  syncProcessor::esyncProcessState syncStateNarrow;
  syncProcessor::esyncProcessState syncState;
  rxSSTVStatusEvent *stce;

  syncWideProc.process();

  syncStateWide=syncWideProc.getSyncState(syncPosition);

  if(syncStateWide!=syncProcessor::MODEDETECT)
    {
      usingWide=true;
      syncNarrowProc.setSyncDetectionEnabled(false);
      syncProcPtr=&syncWideProc;
     }
  else
    {
      syncNarrowProc.process();
      syncStateNarrow=syncNarrowProc.getSyncState(syncPosition);
      if(syncStateNarrow!=syncProcessor::MODEDETECT)
        {
          usingWide=false;
          syncWideProc.setSyncDetectionEnabled(false);
          syncProcPtr=&syncNarrowProc;
        }
      else
        {
          syncProcPtr=NULL;
          syncState=syncProcessor::MODEDETECT;
          if(SSTVState!=HUNTING)
            {
              switchState(HUNTING);
            }
        }
     }
  if(syncProcPtr)
    {
      syncState=syncProcPtr->getSyncState(syncPosition);
    }


  if((syncProcPtr!=0) && (syncProcPtr->getSyncState(syncPosition)==syncProcessor::SYNCLOST))
    {
      switchState(SYNCLOST);
    }
  switch (SSTVState)
    {
    case HUNTING:
      if(syncProcPtr==NULL)
        {
          stce= new rxSSTVStatusEvent(QString("No sync"));
          QApplication::postEvent( dispatcherPtr, stce );  // Qt will delete it when done
          advanceBuffers();
          break; // no sync
        }
      stce= new rxSSTVStatusEvent(QString("Receiving ")+getSSTVModeNameLong(syncProcPtr->getMode()));
      lastUsedModeStr=getSSTVModeNameShort(syncProcPtr->getMode());
      QApplication::postEvent( dispatcherPtr, stce );  // Qt will delete it when done
      // fallthrough for first processing
      switchState(SLANTADJUST); // for logging
      // clear the call received via fskID
      emit (resetCall());

    case SLANTADJUST:
      sampleCounterLatch=syncProcPtr->sampleCounter; //remember where we've got
//      ri=bufferVideoDemod.getReadIndex();
//      addToLog(QString("rxFunctions: sampleCounterLatch= %1,readIndex=%2").arg(sampleCounterLatch).arg(ri),LOGRXFUNC);
      block=(syncPosition)/RXSTRIPE;
      bufferVideoDemod.rewind(syncProcPtr->sampleCounter-block*RXSTRIPE);
//      ri=bufferVideoDemod.getReadIndex();
      //      addToLog(QString("sc_rewind: block=%1,new readIndex= %2").arg(block).arg(ri),LOGRXFUNC);
      syncProcPtr->sampleCounter=block*RXSTRIPE;
      syncProcPtr->currentModePtr->setRxSampleCounter(syncProcPtr->sampleCounter);
      syncProcPtr->currentModePtr->redrawFast(true);
      if(syncProcPtr->currentModePtr->process(bufferVideoDemod.readPointer(),syncPosition-syncProcPtr->sampleCounter,true,syncProcPtr->sampleCounter)!=modeBase::MBRUNNING)
        {
          switchState(END);
          break;
        }


      //    scopeViewerData->addData(SCDATA2,bufferVideoDemod.readPointer(),syncProcPtr->sampleCounter,RXSTRIPE);
      //      addToLog(QString("slant scope add demodIdx=%1; syncProcPtr->sampleCounter=%2").arg(bufferVideoDemod.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
#ifndef QT_NO_DEBUG
      scopeViewerData->addData(SCDATA3,syncProcPtr->currentModePtr->debugStatePtr,syncProcPtr->sampleCounter,RXSTRIPE);
#endif
      addToLog(QString("rxFunctions: currentMode pos:=%1, syncProcPtr->sampleCounter %2").arg(syncPosition-syncProcPtr->sampleCounter).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
      //      addToLog(QString("after Current mode set: %1,syncProcPtr->sampleCounter: %2").arg(rxHoldingBuffer.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
      while(syncProcPtr->sampleCounter<sampleCounterLatch)
        {
          bufferVideoDemod.skip(RXSTRIPE);
          syncProcPtr->sampleCounter+=RXSTRIPE;
          //          addToLog(QString("loop readIndex: %1,syncProcPtr->sampleCounter: %2").arg(rxHoldingBuffer.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
          syncProcPtr->currentModePtr->process(bufferVideoDemod.readPointer(),0,false,syncProcPtr->sampleCounter);
          //      scopeViewerData->addData(SCDATA2,bufferVideoDemod.readPointer(),syncProcPtr->sampleCounter,RXSTRIPE);
#ifndef QT_NO_DEBUG
          scopeViewerData->addData(SCDATA3,syncProcPtr->currentModePtr->debugStatePtr,syncProcPtr->sampleCounter,RXSTRIPE);
#endif
        }
      addToLog(QString("end loop readIndex: %1,syncProcPtr->sampleCounter: %2").arg(bufferVideoDemod.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
      syncProcPtr->currentModePtr->redrawFast(false);
      syncProcPtr->recalculateMatchArray();
      switchState(PROCESSING);
      advanceBuffers();
      break;
    case PROCESSING:
      if(syncState!=syncProcessor::INSYNC)
        {
          switchState(END);
        }
      else if(syncProcPtr->retraceFlag)
        {
          addToLog(QString("retrace detected"),LOGRXFUNC);
          saveImage();
          rewindBuffers(8*RXSTRIPE);
          syncProcPtr->resetRetraceFlag();
          resetParams(false);
          break;
        }
      else
        {
          if(syncProcPtr->currentModePtr->process(bufferVideoDemod.readPointer(),0,false,syncProcPtr->sampleCounter)!=modeBase::MBRUNNING)
            {
              switchState(END);
            }
        }

      if(syncProcPtr->hasNewClock())
        {
          syncProcPtr->currentModePtr->init(syncProcPtr->getNewClock());
          switchState(SLANTADJUST);
        }
#ifndef QT_NO_DEBUG
      scopeViewerData->addData(SCDATA3,syncProcPtr->currentModePtr->debugStatePtr,syncProcPtr->sampleCounter,RXSTRIPE);
#endif
      advanceBuffers();
      if(syncProcPtr->tempOutOfSync)
        {
          bufferIdx=bufferVideoDemod.getReadIndex();
          switchState(WAITFORSYNC);
        }
      break;
    case WAITFORSYNC:
      {

        if(!(syncState==syncProcessor::INSYNC))
          {
            switchState(END);
          }
        else if(syncProcPtr->retraceFlag)
          {
            addToLog(QString("retrace detected"),LOGRXFUNC);
            saveImage();
            //          addToLog(QString("before rewind readIndex:=%1 sampleCounter:=%2").arg(bufferVideoDemod.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
            rewindBuffers(8*RXSTRIPE);
            syncProcPtr->resetRetraceFlag();
            resetParams(false);
            //          addToLog(QString("after resetParms readIndex:=%1 sampleCounter:=%2").arg(bufferVideoDemod.getReadIndex()).arg(syncProcPtr->sampleCounter),LOGRXFUNC);
            break;
          }

        else
          {
            currentIdx=bufferVideoDemod.getReadIndex();
            if(!syncProcPtr->tempOutOfSync)
              {
                bufferVideoDemod.setReadIndex(bufferIdx);
                while(bufferVideoDemod.getReadIndex()!=currentIdx)
                  {
                    if(syncProcPtr->currentModePtr->process(bufferVideoDemod.readPointer(),0,false,syncProcPtr->sampleCounter)==modeBase::MBENDOFIMAGE)
                      {
                        switchState(END);
                      }
                    bufferVideoDemod.skip(RXSTRIPE);
                  }
                if(syncProcPtr->currentModePtr->process(bufferVideoDemod.readPointer(),0,false,syncProcPtr->sampleCounter)==modeBase::MBENDOFIMAGE)
                  {
                    switchState(END);
                  }
                else
                  {
                    switchState(PROCESSING);
                  }
              }
          }
        advanceBuffers();
        break;
      }
    case RESTART:
      resetParams(true);
      break;
    case SYNCLOST:
      saveImage();
      diff=(syncProcPtr->sampleCounter-syncProcPtr->lastValidSyncCounter)/RXSTRIPE;
      addToLog(QString("rewind after synclost %1").arg(diff),LOGRXFUNC);
      rewindBuffers(diff*RXSTRIPE);
      syncProcPtr->resetRetraceFlag();
      resetParams(true);
      break;
    case WAIT:
      break;
    case END:
      saveImage();
      resetParams(false); // will set state to HUNTING
      advanceBuffers();
      break;
    }
}

void sstvRx::saveImage()
{
  bool done=false;
  addToLog("saveImage",LOGRXFUNC);
  endImageSSTVRXEvent *endce;
  if(syncProcPtr->currentModePtr->receivedLines()<(syncProcPtr->currentModePtr->imageLines()*minCompletion)/100)
    {
      endce = new endImageSSTVRXEvent(NOTVALID); //indicate there  is no valid image
    }
  else
    {
//      endce = new endImageSSTVRXEvent(getSSTVModeNameShort(syncProcPtr->getMode()));
      endce = new endImageSSTVRXEvent(syncProcPtr->getMode());
    }
  endce->waitFor(&done);
  QApplication::postEvent(dispatcherPtr, endce);
  while(!done) { usleep(100);}
}


void sstvRx::switchState(eSSTVState  newState)
{
  addToLog(QString("%1 to %2").arg(stateStr[SSTVState]).arg(stateStr[newState]),LOGRXFUNC);
  SSTVState=newState;
}

void sstvRx::eraseImage()
{
  switchState(RESTART);
  while(SSTVState!=HUNTING)
    {
      QApplication::processEvents();
    }
}


#ifndef QT_NO_DEBUG
unsigned int sstvRx::setOffset(unsigned int offset,bool ask)
{
  unsigned int xOffset=0;
  if(ask)
    {
      scopeOffset so;
      so.setOffset(offset);
      if(so.exec()==QDialog::Accepted)
        {
          xOffset=so.getOffset()*1000;
        }
    }
  else
    {
      xOffset=offset*1000;
    }
  syncNarrowProc.clear();
  syncWideProc.clear();
  scopeViewerData->clear();
  syncNarrowProc.setOffset(xOffset);
  syncWideProc.setOffset(xOffset);
  scopeViewerData->setOffset(xOffset);


  return xOffset/1000;
}
#endif

