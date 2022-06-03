#include "drmstatusframe.h"
#include "ui_drmstatusframe.h"
#include "appglobal.h"
#include "drmrx/demodulator.h"
#include <math.h>
#include "configparams.h"

drmStatusFrame::drmStatusFrame(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::drmStatusFrame)
{
  ui->setupUi(this);
  greenPXM=new QPixmap(40,30);
  greenPXM->fill(Qt::green);
  redPXM=new QPixmap(40,30);
  redPXM->fill(Qt::red);
  yellowPXM=new QPixmap(40,30);
  yellowPXM->fill(Qt::yellow);
  init();
}

drmStatusFrame::~drmStatusFrame()
{
  delete ui;
}

void drmStatusFrame::init()
{
  mode="";
  bandwidth=0;
  interleave="";
  protection="";
  qam=0;
  call.clear();
  ui->mscLED->setPixmap(*redPXM);
  ui->facLED->setPixmap(*redPXM);
  ui->frameLED->setPixmap(*redPXM);
  ui->timeLED->setPixmap(*redPXM);

  prevTimeSync=false;
  prevFacValid=0;
  prevFrameSync=false;
  prevMscValid=INVALID;
  prevCurrentSegment=-1;
  prevTransportID=0;
  prevRxSeg=-1;
  prevRobustnessMode=-1;
  QString prevCall="";
  prevSspectrumOccupancy=-1;
  prevProtection=-1;
  prevInterleaverDepth=-1;
  prevMscMode=-1;
  prevBodyTotalSegments=-1;
  prevWMERFAC=-9999;
  prevFreqOff=-9999;
  prevBlockCount=-1;
  drmBusyCount=0;
}




void drmStatusFrame::setStatus()
{
  bool tmp;

  tmp=demodulatorPtr->isTimeSync();
  if(tmp!=prevTimeSync)
    {
      prevTimeSync=tmp;
      if(prevTimeSync)
        {
          ui->timeLED->setPixmap(*greenPXM);
        }
      else
        {
          ui->timeLED->setPixmap(*redPXM);
          if(prevFacValid) ui->facLED->setPixmap(*redPXM);
          if(prevFrameSync) ui->frameLED->setPixmap(*redPXM);
          if(prevMscValid!=INVALID) ui->mscLED->setPixmap(*redPXM);
          prevFacValid=0;
          prevFrameSync=false;
          prevMscValid=INVALID;
          return;
        }
    }
  tmp=demodulatorPtr->isFrameSync();
  if(tmp!=prevFrameSync)
    {
      prevFrameSync=tmp;
      if(tmp)  ui->frameLED->setPixmap(*greenPXM);
      else
        {
          ui->frameLED->setPixmap(*redPXM);
          if(prevFacValid==1) ui->facLED->setPixmap(*redPXM);
          if(prevMscValid!=INVALID) ui->mscLED->setPixmap(*redPXM);
          prevFacValid=0;
          prevMscValid=INVALID;
          return;
        }
    }


  if(prevFacValid!=fac_valid)
    {
      prevFacValid=fac_valid;
      if(fac_valid==1)
        {
          ui->facLED->setPixmap(*greenPXM);
        }
      else
        {
          ui->facLED->setPixmap(*redPXM);
          if(prevMscValid!=INVALID) ui->mscLED->setPixmap(*redPXM);
          prevMscValid=INVALID;

        }
    }
  if(fac_valid==1)
    {
      drmBusyCount=10;
      drmBusy=true;
    }
  else if(drmBusyCount<=0)
    {
      drmBusyCount=0;
      drmBusy=false;
      return;
    }
  else
    {
      drmBusyCount--;
    }

  switch(robustness_mode)
    {
    case 0: mode="A"; break;
    case 1: mode="B"; break;
    case 2: mode="E"; break;
    default: mode=""; break;
    }
  if(mode=="") return;
  if(prevRobustnessMode!=robustness_mode)
    {
      prevRobustnessMode=robustness_mode;
      ui->modeEdit->setText(mode);
    }

  if(callsignValid)
    {
      call=drmCallsign;
      if(prevCall!=call)
        {
          prevCall=call;
          ui->callEdit->setText(call);
          lastReceivedCall=call;
        }
    }

  if(prevMscValid!=msc_valid)
    {
      prevMscValid=msc_valid;
      switch(msc_valid)
        {

        case INVALID:  ui->mscLED->setPixmap(*redPXM); break;
        case VALID: ui->mscLED->setPixmap(*greenPXM); break;
        case ALREADYRECEIVED: ui->mscLED->setPixmap(*yellowPXM); break;
        }
    }

  if(prevSspectrumOccupancy!=spectrum_occupancy_new)
    {
      prevSspectrumOccupancy=spectrum_occupancy_new;
      switch(spectrum_occupancy_new)
        {
        case 0: bandwidth=2.3; break;
        case 1: bandwidth=2.5;; break;
        default:bandwidth=0; break;
        }
      ui->bandwidthEdit->setText(QString::number(bandwidth));
    }


  if(prevProtection!=multiplex_description.PL_PartB)
    {
      prevProtection=multiplex_description.PL_PartB;
      switch (multiplex_description.PL_PartB)
        {
        case 0: protection="High"; break;
        case 1: protection="Low"; break;
        default: ; break;
        }
      ui->protectionEdit->setText(protection);
    }
  if(prevInterleaverDepth!=interleaver_depth_new)
    {
      prevInterleaverDepth=interleaver_depth_new;
      switch(interleaver_depth_new)
        {
        case 0: interleave="Long"; break;
        case 1: interleave="Short"; break;
        default: ; break;
        }
      ui->interleaveEdit->setText(interleave);
    }


  if(prevMscMode!=msc_mode_new)
    {
      prevMscMode=msc_mode_new;
      switch(msc_mode_new)
        {
        case 0: qam=64; break;
        case 1: qam=16; break;
        case 3: qam=4; break;
        default: qam=0; break;
        }
      ui->qamEdit->setText(QString::number(qam));
    }

  if(prevRxSeg!=rxSegments)
    {
      prevRxSeg=rxSegments;
      ui->rxSegmentsEdit->setText(QString::number(rxSegments));
    }
  int tempWMERFAC =round(WMERFAC);
  if(prevWMERFAC!=tempWMERFAC)
    {
      prevWMERFAC=tempWMERFAC;
      ui->snrEdit->setText(QString::number(prevWMERFAC)+" dB");
    }
  if(prevFreqOff!=((int)round(freqOffset-350)))
    {
      prevFreqOff=((int)round(freqOffset-350));
      ui->offsetEdit->setText(QString::number(prevFreqOff)+" Hz");
    }
  if(prevBodyTotalSegments!=bodyTotalSegments)
    {
      prevBodyTotalSegments=bodyTotalSegments;
      ui->totalSegmentsEdit->setText(QString::number(bodyTotalSegments));
      ui->blocksReceivedLabel->setMaxBlocks(bodyTotalSegments);
    }
  if(prevTransportID!=rxTransportID)
    {
      prevTransportID=rxTransportID;
      ui->transportIDEdit->setText(QString::number(prevTransportID));
    }

  if(prevCurrentSegment!=currentSegmentNumber)
    {
      prevCurrentSegment=currentSegmentNumber;
      ui->currentSegmentEdit->setText(QString::number(prevCurrentSegment));
    }
  if(prevBlockCount!=drmBlockList.count())
    {
      prevBlockCount=drmBlockList.count();
      ui->blocksReceivedLabel->setBlocks(drmBlockList);
    }

}


QString modeToString(uint mode)
{
  QString tmp;
  tmp+="Mode: ";
  switch(mode/10000)
    {
    case 0: tmp+="A"; break;
    case 1: tmp+="B"; break;
    case 2: tmp+="E"; break;
    default: tmp+="-"; break;
    }
  tmp+="\nBW: ";
  mode-=(mode/10000)*10000;
  switch(mode/1000)
    {
    case 0: tmp+="2.3"; break;
    case 1: tmp+="2.5";; break;
    default:tmp+="---"; break;
    }
  tmp+="\nProt: ";
  mode-=(mode/1000)*1000;
  switch(mode/100)
    {
    case 0: tmp+="High"; break;
    case 1: tmp+="Low"; break;
    default:tmp+="---" ; break;
    }

  tmp+="\nQAM: ";
  mode-=(mode/100)*100;
  switch(mode/10)
    {
    case 0: tmp+="4"; break;
    case 1: tmp+="16"; break;
    case 2: tmp+="64"; break;
    default: tmp+="--"; break;
    }
  return tmp;
}

QString compactModeToString(uint mode)
{
  QString tmp;
  switch(mode/10000)
    {
    case 0: tmp+="A"; break;
    case 1: tmp+="B"; break;
    case 2: tmp+="E"; break;
    default: tmp+="-"; break;
    }
  tmp+="/"; // bandwidth
  mode-=(mode/10000)*10000;
  switch(mode/1000)
    {
    case 0: tmp+="2.3"; break;
    case 1: tmp+="2.5";; break;
    default:tmp+="---"; break;
    }
  tmp+="/";
  mode-=(mode/1000)*1000;

  switch(mode/100)
    {
    case 0: tmp+="Hi"; break;
    case 1: tmp+="Lo"; break;
    default:tmp+="--" ; break;
    }

  tmp+="/";
  mode-=(mode/100)*100;
  switch(mode/10)
    {
    case 0: tmp+="4"; break;
    case 1: tmp+="16"; break;
    case 2: tmp+="64"; break;
    default: tmp+="--"; break;
    }
  tmp+="/";
  switch(mode&1)
    {
    case 0: tmp+="Long"; break;
    case 1: tmp+="Short"; break;
    default:tmp+="--" ; break;
    }
  return tmp;
}

