#include "syncprocessor.h"
#include "appglobal.h"
#include "sstvparam.h"
#include "configparams.h"
#include "dispatchevents.h"
#include "dispatcher.h"
#include "modes/modes.h"

#ifndef QT_NO_DEBUG
#include "scope/scopeview.h"
#endif

#define LINETOLERANCEMODEDETECT 0.008
#define LINETOLERANCEINSYNC 0.008
#define FILTERDELAYCORRECTION  +9
//#define MINVOLUME 80
#define MINSYNCWIDTHRATIO 0.8
#define MAXSYNCWIDTHRATIO 2


#define NUMBEROFSENSITIVITIES 4

//#define DISABLERETRACE
//#define DISABLEDETECT
#ifdef DISABLENARROW
#pragma message "NARROW DETECTION DISABLED"
#endif


ssenitivity sensitivityArray[NUMBEROFSENSITIVITIES]=
{
  // minMatchedLines maxLineDistanceModeDetect maxLineDistanceInSync onRatio offRatio minVolume maxTempOutOfSyncLines maxOutOfSyncLines;
  {         4,                2,                       7,              0.5,    0.3,   2000,        10, 15},
  {         4,                2,                       7,              0.5  ,  0.3,   1000,        10, 20},
  {         4,                2,                       7,              0.5,    0.3,   100,         15, 30},
  {         4,                2,                       7 ,             0.5,    0.3,   500,         15, 30}
};




const QString syncStateStr[syncProcessor::SYNCVALID+1]=
{
  "SYNCOFF",
  "SYNCUP",
  "SYNCVALID"
};

const QString stateStr[syncProcessor::RETRACEWAIT+1]=
{
  "Mode detect",
  "In Sync",
  "Sync Lost New Mode",
  "Sync Lost False syncs",
  "Sync Lost Missing Lines",
  "Sync Lost",
  "Retrace Wait"
};

/*!
 * \brief syncProcessor::syncProcessor constructor
 *
 * Process the received sync pulses. When a sync pulse is detected, the samplecounter reference is stored in the syncArray.
 * Concurrent with this processing, the VIS decoding and FSKid decoding is done.
 *
 * \param narrow if true then this instance will use narrow settings, else wide settings
 * \param parent
 */



syncProcessor::syncProcessor(bool narrow, QObject *parent) :  QObject(parent),streamDecode(narrow)
{
  detectNarrow=narrow;
  currentModePtr=NULL;
  visMode=NOTVALID;
  maxLineSamples=getMaxLineSamples();
  //  syncFound=false;
  if(!detectNarrow)
  {
    connect(streamDecode.getFskDecoderPtr(),SIGNAL(callReceived(QString)),SLOT(slotNewCall(QString)));
    connect(streamDecode.getVisDecoderPtr(),SIGNAL(visCodeWideDetected(int,uint)),SLOT(slotVisCodeDetected(int,uint)));
  }
  else
  {
    connect(streamDecode.getVisDecoderPtr(),SIGNAL(visCodeNarrowDetected(int,uint)),SLOT(slotVisCodeDetected(int,uint)));
  }
}

syncProcessor::~syncProcessor()
{
  int i;
  for(i=0;i<=ENDNARROW;i++)
  {
    matchArray[i].clear();
  }
  if(currentModePtr!=NULL) delete currentModePtr;
}

void syncProcessor::reset()
{
  sampleCounter=0;
  init();
  streamDecode.reset();

#ifndef QT_NO_DEBUG
  scopeViewerSyncNarrow->setCurveName("SYNC VOL",SCDATA1);
  scopeViewerSyncNarrow->setCurveName("INPUT VOL",SCDATA2);
  scopeViewerSyncNarrow->setCurveName("SYNC STATE",SCDATA3);
  scopeViewerSyncNarrow->setCurveName("FREQ",SCDATA4);
  scopeViewerSyncNarrow->setAxisTitles("Samples","int","State");

  scopeViewerSyncWide->setCurveName("SYNC VOL",SCDATA1);
  scopeViewerSyncWide->setCurveName("INPUT VOL",SCDATA2);
  scopeViewerSyncWide->setCurveName("SYNC STATE",SCDATA3);
  scopeViewerSyncWide->setCurveName("FREQ",SCDATA4);
  scopeViewerSyncWide->setAxisTitles("Samples","int","State");
#endif

}

/**
 * @brief initialize sync processor
 * idxStart and idxEnd are set for narrow or wide modes in Auto mode, else idxStart=idxEnd= selected mode
 * enableSyncDetection is set to false for AVT modes.
 *
 */
void syncProcessor::init()
{
  enableSyncDetection=true;
  //  syncAvg=0;

  if(detectNarrow)
  {
    idxStart=STARTNARROW;
    idxEnd=ENDNARROW;
  }
  else
  {
    idxStart=STARTWIDE;
    idxEnd=ENDWIDE;
  }

  if(sstvModeIndexRx!=0)
  {
    //      if((sstvModeIndexRx>=idxStart) && (sstvModeIndexRx<=idxEnd))
    {
      idxEnd=idxStart=(esstvMode)(sstvModeIndexRx-1);
      if((idxStart>=AVT24)&&(idxStart<=AVT94))
      {
        enableSyncDetection=false;
      }
    }
  }
  visMode=NOTVALID; // and reset the visMode
  syncProcesState=MODEDETECT;
  modifiedClock=rxClock/SUBSAMPLINGFACTOR;
  syncArrayIndex=0;
  syncArray[0].init();
  retraceFlag=false;
  syncState=SYNCOFF;
  displaySyncEvent* ce;
  ce = new displaySyncEvent(0); // reset sync vuMeter
  QApplication::postEvent(dispatcherPtr, ce);
  addToLog("init called",LOGSYNCSTATE);
  clearMatchArray();
  currentMode=NOTVALID;
  lineTolerance=LINETOLERANCEMODEDETECT;
  minMatchedLines=sensitivityArray[sensitivity].minMatchedLines;
}

/**
 * @brief process the data buffer to extract syncs
 *
 *
 * It calls extract sync and the streamDecode to extarct FSKid and VIS Code
 *
 */


void syncProcessor::process()
{
#ifdef DISABLENARROW
  if(!detectNarrow)
  {
#endif

    streamDecode.process(freqPtr,sampleCounter); // only VIS and FSK if not narrow
    extractSync();
#ifdef DISABLENARROW
  }
#endif

#ifndef QT_NO_DEBUG
  if(detectNarrow)
  {
    scopeViewerSyncNarrow->addData(SCDATA1,syncVolumePtr,sampleCounter,RXSTRIPE);
    //      scopeViewerSyncNarrow->addData(SCDATA2,inputVolumePtr,sampleCounter,RXSTRIPE);
    scopeViewerSyncNarrow->addData(SCDATA2,inputVolumePtr,sampleCounter,RXSTRIPE);
    scopeViewerSyncNarrow->addData(SCDATA3,syncStateBuffer,sampleCounter,RXSTRIPE);
    scopeViewerSyncNarrow->addData(SCDATA4,freqPtr,sampleCounter,RXSTRIPE);
  }
  else
  {
    scopeViewerSyncWide->addData(SCDATA1,syncVolumePtr,sampleCounter,RXSTRIPE);
    //      scopeViewerSyncWide->addData(SCDATA2,syncAvgPtr,sampleCounter,RXSTRIPE);
    scopeViewerSyncWide->addData(SCDATA2,inputVolumePtr,sampleCounter,RXSTRIPE);
    scopeViewerSyncWide->addData(SCDATA3,syncStateBuffer,sampleCounter,RXSTRIPE);
    scopeViewerSyncWide->addData(SCDATA4,freqPtr,sampleCounter,RXSTRIPE);
  }
#endif
}


/**
 * @brief extract the syncs from the data
 *
 * The function fills in the syncArray en calls validateSync.
 * If syncProcesState==INSYNC then the out of sync is tracked
 * The last check is to see if we received a valid VIS code
 *
 */

void syncProcessor::extractSync()
{
  int i;
  int lastSync;
  for(i=0;i<RXSTRIPE;i++)
  {
    switch(syncState)
    {
    case SYNCVALID:
    case SYNCOFF:
      if(inputVolumePtr[i]<sensitivityArray[sensitivity].minVolume) break;
      if(syncVolumePtr[i]>sensitivityArray[sensitivity].onRatio*inputVolumePtr[i])
      {
        syncArray[syncArrayIndex].start=sampleCounter+i;
        syncArray[syncArrayIndex].startVolume=syncVolumePtr[i];
        switchSyncState(SYNCACTIVE,sampleCounter+i);
      }
      break;
    case SYNCACTIVE:
      if(inputVolumePtr[i]<sensitivityArray[sensitivity].minVolume)
      {
        switchSyncState(SYNCOFF,sampleCounter+i);
        break;
      }
      if(syncVolumePtr[i]<sensitivityArray[sensitivity].offRatio*inputVolumePtr[i])
      {
        syncArray[syncArrayIndex].end=sampleCounter+i;
        if(validateSync())
        {
          switchSyncState(SYNCVALID,sampleCounter+i);
        }
        else
        {
          switchSyncState(SYNCOFF,sampleCounter+i);
        }
      }
      break;
    }
#ifndef QT_NO_DEBUG
    syncStateBuffer[i]=(unsigned char)syncState*STATESCALER;
#endif
  }

  if((syncProcesState==INSYNC) && (enableSyncDetection))
  {
    if(activeChainPtr==NULL)
    {
      switchProcessState(SYNCLOST);
    }
    else
    {
      lastSync=syncArray[activeChainPtr->last()->to].end;
      if((sampleCounter+RXSTRIPE-RXSTRIPE/7)>(lastSync+sensitivityArray[sensitivity].maxTempOutOfSyncLines*samplesPerLine))
      {
        if(sensitivity!=(NUMBEROFSENSITIVITIES-1)) tempOutOfSync=true; // no temp out of sync if DX
      }
      missingLines=(uint) round(((sampleCounter+RXSTRIPE-RXSTRIPE/7)-(lastSync+samplesPerLine))/samplesPerLine+1);
      calcSyncQuality();
    }
  }
  if((visMode>=AVT24)&&(visMode<=AVT94))
  {
    currentMode=visMode;
    enableSyncDetection=false;
    createModeBase();
    visMode=NOTVALID;
    switchProcessState(INSYNC);
  }
}

/**
 * @brief validate sync and build up syncArray
 *
 * The function evaluates the sync pulse width and sets the retrace flag
 * *
 * \retval false if no valid sync and no further processing is neeeded
 *         true if a new syncArray entry is added and further processing is necessary
 */


bool syncProcessor::validateSync()
{
  bool result;
#ifndef DISABLERETRACE
  if(syncArray[syncArrayIndex].diffStartEnd()>=MINRETRACEWIDTH)
  {
    syncArray[syncArrayIndex].retrace=true; // simply set retrace true
    result=true;
  }
  else if(syncArray[syncArrayIndex].diffStartEnd()>=MINRETRACEWIDTH/4)
  {
    if((syncArrayIndex>2)
       && (syncArray[syncArrayIndex].end-syncArray[syncArrayIndex-1].start)>=MINRETRACEWIDTH
       && (syncArray[syncArrayIndex].start-syncArray[syncArrayIndex-1].end)<=MINRETRACEWIDTH/5)
    {
      syncArray[syncArrayIndex-1].end=syncArray[syncArrayIndex].end;
      syncArray[syncArrayIndex-1].retrace=true;
      syncArray[syncArrayIndex-1].diffStartEnd(); // just calculate the width;
      syncArrayIndex--;
    }
    result=true;
  }
  else if((syncArray[syncArrayIndex].diffStartEnd()>=0.004*SAMPLERATE) && (syncArray[syncArrayIndex].diffStartEnd()<=0.025*SAMPLERATE))
#else
  if((syncArray[syncArrayIndex].diffStartEnd()>=0.004*SAMPLERATE) && (syncArray[syncArrayIndex].diffStartEnd()<=0.025*SAMPLERATE))
#endif
  {
    syncArray[syncArrayIndex].retrace=false;
    //      addToLog(QString("index %1, mid:=%2").arg(syncArrayIndex).arg(syncArray[syncArrayIndex].mid),LOGSYNCACCEPTED);
    result=true;
  }
  else
  {
    //      addToLog(QString("Sync rejected:%1 end:%2 width %3").arg(syncArray[syncArrayIndex].start)
    //               .arg(syncArray[syncArrayIndex].end).arg(syncArray[syncArrayIndex].diffStartEnd()),LOGSYNCREJECTED);
    result=false;
  }

  if(result)
  {
    checkSyncArray();
    switch(syncProcesState)
    {
    case MODEDETECT:
      if(findMatch())
      {
        visMode=NOTVALID; //reset visMode;
#ifndef DISABLEDETECT
        //we have a new mode
        if(!createModeBase())
        {
          addToLog("Error creating modeBase",LOGALL);
          result=false;
        }
        else
        {
          falseSyncs=0;
          lineTolerance=LINETOLERANCEINSYNC;
          // when we have S1,S2 or SDX then we have to set the syncposition at the beginning of the green line
          //          syncPosition=currentModePtr->adjustSyncPosition(syncArray[0].end)- FILTERDELAYCORRECTION; // type 1 sync end
          unsigned int syncCorrected;;
          syncWidth=getSyncWidth(currentMode ,modifiedClock);

          if(syncArray[0].retrace)
          {
            syncCorrected=syncArray[0].end;
          }
          else
          {
            syncCorrected=(syncArray[0].start+syncArray[0].end)/2+syncWidth/2;
          }

          syncPosition=currentModePtr->adjustSyncPosition(syncCorrected,syncArray[0].retrace)+FILTERDELAYCORRECTION; // type 2 sync end
          tempOutOfSync=false;
          slantAdjustLine=6;
          slantAdjust(true);
          switchProcessState(INSYNC);
          addToLog(QString("Mode detected: %1, sync at %2").arg(getSSTVModeNameShort(currentMode)).arg(syncPosition),LOGSYNCEVAL);
        }
#endif
      }
      break;
    case INSYNC:
      if(enableSyncDetection) trackSyncs();
      break;
    case SYNCLOSTNEWMODE:
    case SYNCLOSTFALSESYNC:
    case SYNCLOSTMISSINGLINES:
    case SYNCLOST:
      addToLog("synclost detected",LOGSYNCSTATE);
      break;
    case RETRACEWAIT:
      break;

    }
    syncArrayIndex++;
    syncArray[syncArrayIndex].init();
  }
  return result;
}


void  syncProcessor::trackSyncs()
{
  if(activeChainPtr==NULL)
  {
    return;
  }
  if(activeChainPtr->count()==0)
  {
    return;
  }
  if(addToChain(currentMode,activeChainPtr->last()->to))
  {
    falseSyncs=0;
    missingLines=0;
    if(tempOutOfSync)
    {
      // check if we can resynchronize
      if(activeChainPtr->last()->lineSpacing<=2)
      {
        tempOutOfSync=false;
      }
    }
  }

  else
  {
    falseSyncs++;
  }
  if(retraceFlag)
  {
    switchProcessState(RETRACEWAIT);
    return;
  }
  if(currentModeMatchChanged)
  {
    currentModeMatchChanged=false;
    calculateLineNumber(activeChainPtr->last()->from,activeChainPtr->last()->to);
    lastSyncTest=activeChainPtr->last()->endTo;
    slantAdjust(false);
    if(falseSlantSync>=10)
    {
      switchProcessState(SYNCLOSTFALSESYNC);
    }
  }
}

void syncProcessor::slotNewCall(QString call)
{
  emit callReceived(call);
  retraceFlag=true;
}

void syncProcessor::slotVisCodeDetected(int mode,uint visSampleCounter)
{
  if((mode>=idxStart) && (mode<=idxEnd))
  {
    visMode=(esstvMode)mode;
    if((visMode>=AVT24)&&(visMode<=AVT94))
    {
      enableSyncDetection=false;
    }
    addToLog(QString("viscode %1 accepted").arg(visMode),LOGSYNCACCEPTED);
    idxStart=idxEnd=(esstvMode)mode;
    minMatchedLines=3;
    visEndCounter=visSampleCounter;
  }
  else
  {
    visMode=NOTVALID;
  }
}

void  syncProcessor::calcSyncQuality()
{
  int k;
  quint16 fs=0;
  quint16 tmp;
  syncQuality=10;
  QString str;
  //calc missing syncs in the last 10
  if(activeChainPtr->count()>=10)
  {
    for(k=activeChainPtr->count()-10;k<activeChainPtr->count()-1;k++)
    {
      tmp=activeChainPtr->at(k)->to-activeChainPtr->at(k)->from-1;
      tmp/=activeChainPtr->at(k)->lineSpacing; // this gives us the number of false syncs per line
      fs+=tmp;
    }
  }
  if(missingLines>5)
  {
    str=QString("FS: %1,%2 ").arg(fs).arg(((fs*3)/25));
    syncQuality-=((fs*3)/25);
  }
  if(missingLines>=sensitivityArray[sensitivity].maxOutOfSyncLines)
  {
    str+=QString("missingLines: %1 ").arg(missingLines);
    syncQuality=-1;
  }
  syncQuality-=(falseSlantSync*2);
  str+=QString("falseSlantSync: %1").arg(falseSlantSync);
  if(syncQuality<0) syncQuality=0;
  if((syncQuality<=0) && (sensitivity!=(NUMBEROFSENSITIVITIES-1)))  // i.e DX Mode
  {

    addToLog(QString("syncQuality SYNCLOST %1").arg(str),LOGSYNCQUALITY);
    switchProcessState(SYNCLOST);
  }
}

/**
 * @brief syncProcessor::calculateLineNumber
 * @param fromIdx
 * @param toIdx
 */

void  syncProcessor::calculateLineNumber(uint fromIdx,uint toIdx)
{
  quint16 lnbr;
  double fract;
  lineCompare(samplesPerLine,0,toIdx,lnbr,fract);
  syncArray[toIdx].length=syncArray[toIdx].end-syncArray[fromIdx].end;
  syncArray[toIdx].lineNumber=lnbr;
  lastUpdatedSync=toIdx;
}

void syncProcessor::checkSyncArray()
{

  if(syncArray[syncArrayIndex].retrace)
  {
    syncArray[0]=syncArray[syncArrayIndex];
    syncArrayIndex=0;
    //      syncPosition=syncArray[0].end + FILTERDELAYCORRECTION;
    addToLog(QString("Found retrace: start:%1 end:%2  width %3").arg(syncArray[0].start).arg(syncArray[0].end).arg(syncArray[0].end-syncArray[0].start),LOGSYNCACCEPTED);
    if(syncProcesState==INSYNC)
    {
      retraceFlag=true;
      syncProcesState=RETRACEWAIT;
    }
  }

  if(syncArrayIndex>=(MAXSYNCENTRIES-1))
  {
    //shift syncArray
    dropTop();
  }
  addToLog(QString("Found sync: start:%1 end:%2 width %3 at %4").arg(syncArray[syncArrayIndex].start).arg(syncArray[syncArrayIndex].end).arg(syncArray[syncArrayIndex].end-syncArray[syncArrayIndex].start).arg(syncArrayIndex),LOGSYNCACCEPTED);
}


/**
 * @brief syncProcessor::findMatch
 The syncArray is filled with valid synPositions.
 idxStart and idxEnd indicate the sstv modes to compare the sync pulse intervals with.


* @return true if match

*/


bool syncProcessor::findMatch()
{
  int i,j,k,m;
  int fs;
  uint minTotLines=9999;
  double minFract=1;
  int idx=-1;
  QList<modeMatchList *> changeList;
  QList <int> modeList;
  QList<uint> totalLinesList;
  QList<double> totalFractList;

  if(enableSyncDetection)
  {
    //      addToLog (QString(" checking match for syncArrayIndex %1 at %2").arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
    for(i=idxStart;i<=idxEnd;i++)
    {
      syncWidth=getSyncWidth((esstvMode)i ,modifiedClock);
      if(addToMatch((esstvMode)i))
      {
        for(j=0;j<matchArray[i].count();j++)
        {
          if(matchArray[i][j]->count()>=(int)minMatchedLines)
          {
            fs=0;
            for(k=0;k<matchArray[i][j]->count()-1;k++)
            {
              fs+=matchArray[i][j]->at(k)->to-matchArray[i][j]->at(k)->from-1;
            }
            if (fs<20)
            {
              changeList.append(matchArray[i][j]);
              modeList.append(i);
            }
          }

        }
      }
    }
    for (m=0;m<changeList.count();m++)
    {
      totalLinesList.append(calcTotalLines( changeList.at(m)));
      totalFractList.append(calcTotalFract( changeList.at(m)));
    }
    for (m=0;m<changeList.count();m++)
    {
      if((minTotLines>=totalLinesList.at(m)) && (totalFractList.at(m)<=minFract))
      {
        idx=m;
        minTotLines=totalLinesList.at(m);
        minFract=totalFractList.at(m);
      }
    }


    if(idx>=0)
    {
      if(visMode!=NOTVALID)
      {
        currentMode=visMode;
      }
      else
      {
        currentMode=(esstvMode)modeList.at(idx);
      }

      activeChainPtr=changeList.at(idx);
      samplesPerLine=getLineLength(currentMode,modifiedClock);
      cleanupMatchArray();
      return true;
    }
    return false;
  }

  return false;
}


uint syncProcessor::calcTotalLines(modeMatchList *mlPtr)
{
  int i;
  uint lines=0;
  for(i=0;i<mlPtr->count();i++)
  {
    lines+=mlPtr->at(i)->lineSpacing;
  }
  return lines;
}

double syncProcessor::calcTotalFract(modeMatchList *mlPtr)
{

  int i;
  double fract=0;
  for(i=0;i<mlPtr->count();i++)
  {
    fract+=mlPtr->at(i)->fraction;
  }
  return fract;

}

/**
 * @brief syncProcessor::addToMatch
 * @param mode sstv mode to be evaluated
 * @return true if added to chain
 */

bool syncProcessor::addToMatch(esstvMode mode)
{
  int  i;
  
  if(syncArrayIndex<1) return false;
  for(i=syncArrayIndex-1;i>=0;i--)
  {

    if(addToChain(mode,i))
    {
      return true;
    }
  }
  return false;
}






bool syncProcessor::addToChain(esstvMode mode,  uint fromIdx)
{
  int i;
  double fract;
  quint16 lineSpacing;

  samplesPerLine=getLineLength(mode,modifiedClock);
  
  if(!lineCompare(samplesPerLine,fromIdx,syncArrayIndex,lineSpacing,fract))
  {
    return false;
  }


  if((syncArray[syncArrayIndex].diffStartEnd()<syncWidth*MINSYNCWIDTHRATIO)
     || (syncArray[syncArrayIndex].diffStartEnd()>syncWidth*MAXSYNCWIDTHRATIO)
     || (syncArray[fromIdx].diffStartEnd()<syncWidth*MINSYNCWIDTHRATIO))
  {

    return false;
  }

  if (syncArray[fromIdx].diffStartEnd()>syncWidth*MAXSYNCWIDTHRATIO)
  {
    if(fromIdx!=0 || !syncArray[0].retrace)
    {
      return false;
    }
  }
  if(syncProcesState==MODEDETECT)
  {
    if(lineSpacing>sensitivityArray[sensitivity].maxLineDistanceModeDetect)
    {
      return false;
    }
  }

  bool found=false;
  if(matchArray[mode].count()==0) // we don't have a chain yet
  {
    matchArray[mode].append(new modeMatchList);
    matchArray[mode][0]->append(new smatchEntry(fromIdx,syncArrayIndex,lineSpacing,fract,syncArray[fromIdx].end,syncArray[syncArrayIndex].end));
    //      addToLog(QString("Match: mode=%1,new chain=%2 syncIndex=%3 end=%4").arg(getSSTVModeNameShort(mode)).arg(matchArray[mode].count()-1).arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
  }
  else
  {
    // can we append this to an existing chain?
    for(i=0;i<matchArray[mode].count();i++)
    {
      if(matchArray[mode][i]->last()->to==fromIdx)
      {
        if((syncProcesState==INSYNC) && (i==0) )
        {
          currentModeMatchChanged=true;
        }
        matchArray[mode][i]->append(new smatchEntry(fromIdx,syncArrayIndex,lineSpacing,fract,syncArray[fromIdx].end,syncArray[syncArrayIndex].end));
        //              addToLog(QString("Match: mode=%1,chain=%2 syncIndex=%3 end=%4").arg(getSSTVModeNameShort(mode)).arg(i).arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
        found=true;
        break;
      }
    }
    if(!found)
    {
      matchArray[mode].append(new modeMatchList);
      matchArray[mode].last()->append(new smatchEntry(fromIdx,syncArrayIndex,lineSpacing,fract,syncArray[fromIdx].end,syncArray[syncArrayIndex].end));
      //          addToLog(QString("Match: mode=%1,new chain=%2 syncIndex=%3 end=%4").arg(getSSTVModeNameShort(mode)).arg(matchArray[mode].count()-1).arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
    }

  }
  return true;
}


void syncProcessor::clearMatchArray()
{
  int i,j;
  for(i=idxStart;i<=idxEnd;i++)
  {
    for(j=0;j<matchArray[i].count();j++)
    {
      removeMatchArrayChain((esstvMode)i,0);
    }
    matchArray[i].clear();
  }
  activeChainPtr=NULL;
  switchProcessState(MODEDETECT);
}

void syncProcessor::removeMatchArrayChain(esstvMode mode,int chainIdx)
{
  int i;
  for(i=0;i<matchArray[mode][chainIdx]->count();i++)
  {
    delete matchArray[mode][chainIdx]->at(i);
  }
  matchArray[mode][chainIdx]->clear();
  delete matchArray[mode][chainIdx];
  matchArray[mode].takeAt(chainIdx);
}

void syncProcessor::cleanupMatchArray()
{
  int i,j;
  //  double fract=0;
  //  quint16 lnbr=0;
  for(i=idxStart;i<=idxEnd;i++)
  {
    for(j=0;j<matchArray[i].count();)
    {
      if(activeChainPtr!=matchArray[i][j])
      {
        removeMatchArrayChain((esstvMode)i,j);
      }
      else
      {
        j++;
      }
    }
  }

  for(i=0;i<activeChainPtr->count();i++)
  {
    syncArray[activeChainPtr->at(i)->from].inUse=true;
    syncArray[activeChainPtr->at(i)->to].inUse=true;
  }
  for(i=0;i<syncArrayIndex;)
  {
    if(!syncArray[i].inUse)
    {
      deleteSyncArrayEntry(i);
    }
    else
    {
      i++;
    }
  }
  for(i=0;i<syncArrayIndex;i++)
  {
    calculateLineNumber(i,i+1);
  }
}

void syncProcessor::dropTop()
{
  deleteSyncArrayEntry(0);
}

void syncProcessor::deleteSyncArrayEntry(uint entry)
{
  int i,j,k;
  modeMatchList *ml;
  //  smatchEntry *tempPtr;
  if(entry>=syncArrayIndex) return;
  // delete or adapt the matchArrays
  for(i=idxStart;i<=idxEnd;i++)   //all modes
  {
    for(j=0;j<matchArray[i].count();) //all chains
    {
      for(k=0;k<matchArray[i][j]->count();)
      {
        ml=matchArray[i][j];
        if (ml->at(k)->from==entry)
        {
          delete matchArray[i][j]->takeAt(k);
        }
        else
        {
          if(ml->at(k)->from>entry)
          {
            ml->at(k)->from--;
          }
          if(ml->at(k)->to>entry)
          {
            ml->at(k)->to--;
          }
          k++;
        }
      }
      if(matchArray[i][j]->count()==0)
      {
        if(activeChainPtr==matchArray[i][j])
        {
          activeChainPtr=NULL;
        }
        delete matchArray[i].takeAt(j);
      }
      else
      {
        j++;
      }
    }
  }
  for(i=entry;i<(int)syncArrayIndex;i++)
  {
    syncArray[i]=syncArray[i+1];
  }
  syncArrayIndex--;
}


/**
 * @brief syncProcessor::recalculateMatchArray
 * \todo recalculate
 */

void syncProcessor::recalculateMatchArray()
{
}


/**
 * @brief syncProcessor::lineCompare
 * @param samPerLine samples per line
 * @param srcIdx from sync array entry
 * @param dstIdx to sync array entry
 * @param lineNumber number of lines
 * @param fraction fractional part
 * @return true if a match for that line (i.e fractional part is smaller than lineTolerance
 */


bool syncProcessor::lineCompare(DSPFLOAT samPerLine, int srcIdx, int dstIdx, quint16 &lineNumber, double &fraction)
{
  double delta;
  delta=(double)(syncArray[dstIdx].end-syncArray[srcIdx].end);

  lineNumber=(delta+samPerLine/2.) /samPerLine;
  fraction=(double)lineNumber-delta/samPerLine;
  if(fraction<0) fraction=-fraction;
  //  if(fraction<lineTolerance)
  //  addToLog(QString("Lnbr: %1, fract: %2, delta: %3 src: %4,dest: %5, OK %6")
  //           .arg(lineNumber).arg(fraction).arg(delta).arg(srcIdx).arg(dstIdx).arg(fraction<lineTolerance)
  //           ,LOGSYNCCOMP);
  return (fraction<lineTolerance);
}

void syncProcessor::resetRetraceFlag()
{
  retraceFlag=false;
  clearMatchArray();
}


bool  syncProcessor::createModeBase()
{
  bool done=false;
  if(currentModePtr) delete currentModePtr;
  currentModePtr=NULL;
  switch (currentMode)
  {
  case M1:
  case M2:
    currentModePtr=new modeGBR(currentMode,RXSTRIPE,false,false);
    break;
  case S1:
  case S2:
  case SDX:
    currentModePtr=new modeGBR2(currentMode,RXSTRIPE,false,false);
    break;
  case R36:
    currentModePtr=new modeRobot1(currentMode,RXSTRIPE,false,false);
    break;
  case R24:
  case R72:
  case MR73:
  case MR90:
  case MR115:
  case MR140:
  case MR175:
  case ML180:
  case ML240:
  case ML280:
  case ML320:
    currentModePtr=new modeRobot2(currentMode,RXSTRIPE,false,false);
    break;
  case SC2_60:
  case SC2_120:
  case SC2_180:
  case P3:
  case P5:
  case P7:
  case MC110N:
  case MC140N:
  case MC180N:
    currentModePtr=new modeRGB(currentMode,RXSTRIPE,false,false);
    break;
  case FAX480:
  case BW8:
  case BW12:
    currentModePtr=new modeBW(currentMode,RXSTRIPE,false,false);
    break;
  case AVT24:
  case AVT90:
  case AVT94:
    currentModePtr=new modeAVT(currentMode,RXSTRIPE,false,false);
    break;
  case PD50:
  case PD90:
  case PD120:
  case PD160:
  case PD180:
  case PD240:
  case PD290:
  case MP73:
  case MP115:
  case MP140:
  case MP175:
    currentModePtr=new modePD(currentMode,RXSTRIPE,false,false);
    break;
  case MP73N:
  case MP110N:
  case MP140N:
    currentModePtr=new modePD(currentMode,RXSTRIPE,false,true);
    break;
  default:
    currentMode=NOTVALID;
    break;
  }
  if (currentMode!=NOTVALID)
  {
    initializeSSTVParametersIndex(currentMode,false);
    QString s=getSSTVModeNameLong(currentMode);
    addToLog(QString("create RX mode: %1").arg(getSSTVModeNameShort(currentMode)),LOGSYNCSTATE);
    currentModePtr->init(modifiedClock);
    startImageRXEvent* ce = new startImageRXEvent(QSize(currentModePtr->imagePixels(),currentModePtr->imageLines()));
    ce->waitFor(&done);
    QApplication::postEvent(dispatcherPtr, ce);
    while(!done)
    {
      QApplication::processEvents();
    }
  }
  return (currentMode!=NOTVALID);
}


void syncProcessor::regression(DSPFLOAT &a,DSPFLOAT &b,bool initial)
{
  /* calculate linear regression
    formula x=a+by
    b=sum((x[i]-xm)*(y[i]-ym))/sum((y[i]-ym)*(y[i]-ym))
    a=xm-b*ym
  */
  int j;

  int count=activeChainPtr->count();
  falseSlantSync=0;
  DSPFLOAT sum_x,sum_y,sum_xx,sum_xy;
  sum_x=sum_y=sum_xx=sum_xy=a=b=0;
  unsigned int endZero;
  unsigned int tempCount=0;
  j=0;
  endZero=syncArray[0].end;
  for(;j<count;j++)
  {

    if((activeChainPtr->at(j)->fraction>0.006)&&(!initial))
    {
      continue;
    }

    slantXYArray[tempCount].y=(DSPFLOAT)(syncArray[activeChainPtr->at(j)->to].end-endZero);
    slantXYArray[tempCount].x= syncArray[activeChainPtr->at(j)->to].lineNumber*samplesPerLine;
    addToLog(QString("pos: %1, x=%2 y=%3 syncIndex:%4, diff %5").arg(tempCount).arg(slantXYArray[tempCount].x).arg(slantXYArray[tempCount].y).arg(activeChainPtr->at(j)->to).arg(slantXYArray[tempCount].x-slantXYArray[tempCount].y) ,LOGSLANT);
    if((fabs(slantXYArray[tempCount].x-slantXYArray[tempCount].y)>150.)&&(!initial))
    {
      falseSlantSync++;
      continue;
    }
    sum_x+=slantXYArray[tempCount].x;
    sum_y+=slantXYArray[tempCount].y;
    sum_xx+=slantXYArray[tempCount].x*slantXYArray[tempCount].x;
    sum_xy+=slantXYArray[tempCount].x*slantXYArray[tempCount].y;
    lastValidSyncCounter=syncArray[activeChainPtr->at(j)->to].end;
    tempCount++;
  }
  b=((tempCount)*sum_xy-(sum_x*sum_y))/((tempCount)*sum_xx-(sum_x*sum_x));
  a=sum_y/(tempCount)-(b*sum_x)/(tempCount);
}



bool syncProcessor::slantAdjust(bool initial)
{
  DSPFLOAT a,b;
  if ((currentMode>=AVT24) && (currentMode <= AVT94)) return true;
  if(currentMode==NOTVALID) return true;
  falseSlantSync=0;
  if(!initial)
  {
    if(syncArray[activeChainPtr->last()->to].lineNumber<slantAdjustLine) return false;
  }

  regression(a,b,initial);

  //  addToLog(QString("regr. params line %1 a:%2 b:%3").arg(slantAdjustLine).arg(a).arg(b),LOGSLANT);
  slantAdjustLine+=5;
  if(!autoSlantAdjust) return false;
  if(initial)
  {
    if((fabs(1.-b)>0.02) || (fabs(a)>100)) return false;
  }
  else
  {
    if((fabs(1.-b)>0.005) || (fabs(a)>50)) return false;
  }
  if  (((fabs(1.-b)>0.00001)  || (fabs(a)>1)  )    && autoSlantAdjust)
  {
    newClock=true;
    modifiedClock*=b;
    samplesPerLine=getLineLength(currentMode,modifiedClock); //recalculate the samples per line
    addToLog(QString("new clock accepted: %1 a=%2,b=%3").arg(modifiedClock).arg(a).arg(b),LOGSLANT);

    syncArray[0].end+=(long)round(a);
    syncArray[0].start+=(long)round(a);

    unsigned int syncCorrected;
    if(syncArray[0].retrace)
    {
      syncCorrected=syncArray[0].end;
    }
    else
    {
      syncCorrected=(syncArray[0].start+syncArray[0].end)/2+getSyncWidth(currentMode,modifiedClock)/2;
    }

    syncPosition=currentModePtr->adjustSyncPosition(syncCorrected,syncArray[0].retrace)+FILTERDELAYCORRECTION; // type 2 sync end

    recalculateMatchArray();
    addToLog(QString("slantAdjust: modified  syncpos:=%1").arg(syncPosition),LOGSLANT);

    return true;
  }
  return false;
}



void syncProcessor::switchSyncState(esyncState newState,quint32 sampleCntr)
{
  Q_UNUSED(sampleCntr)
  if(syncState!=newState)
  {
    addToLog(QString("switching from %1 to %2 at %3").arg(syncStateStr[syncState]).arg(syncStateStr[newState]).arg(sampleCntr),LOGSYNCSTATE);
    syncState=newState;
  }
}

void syncProcessor::switchProcessState(esyncProcessState  newState)
{
  addToLog(QString("syncProcessState %1 to %2").arg(stateStr[syncProcesState]).arg(stateStr[newState]),LOGSYNCPROCESSSTATE);
  if((newState==SYNCLOSTFALSESYNC) || (newState==SYNCLOSTNEWMODE)||(newState==SYNCLOSTMISSINGLINES) || (newState==SYNCLOST))
  {
    newState=SYNCLOST;
  }
  syncProcesState=newState;
}

#ifndef QT_NO_DEBUG
void syncProcessor::setOffset(unsigned int dataScopeOffset)
{
  xOffset=dataScopeOffset;
  scopeViewerSyncNarrow->setOffset(xOffset);
  scopeViewerSyncWide->setOffset(xOffset);
}

void syncProcessor::clear()
{
  scopeViewerSyncNarrow->clear();
  scopeViewerSyncWide->clear();
  xOffset=0;
  scopeViewerSyncNarrow->setOffset(xOffset);
  scopeViewerSyncWide->setOffset(xOffset);
}


#endif
