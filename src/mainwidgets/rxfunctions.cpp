#include "rxfunctions.h"
#include "appglobal.h"
#include "configparams.h"
#include "drmrx.h"
#include "soundbase.h"
#include "dispatcher.h"
#include "rxwidget.h"
#include "sstvrx.h"

#include <QApplication>



const QString rxStateStr[rxFunctions::RXINIT+1]=
{
  "IDLE",
  "RUNNING",
  "RESTART",
  "INIT"
};

rxFunctions::rxFunctions(QObject *parent) : QThread(parent)
{
  rxState=RXIDLE;
  sstvRxPtr=new sstvRx;
  drmRxPtr=new drmRx;
  rxBytes=0;
  setObjectName("rx-thread");
}

rxFunctions::~rxFunctions()
{
  delete sstvRxPtr;
  delete drmRxPtr;
}

//static DSPFLOAT dummyBuf[RXSTRIPE];

void rxFunctions::run()
{
  int count;
  DSPFLOAT tempBuf[RXSTRIPE];
  DSPFLOAT volBuf[RXSTRIPE];
  abort=false;
  while(!abort)
    {
      switch(rxState)
        {
        case RXIDLE:
          msleep(200);
          break;
        case RXRUNNING:
          if((count=soundIOPtr->rxBuffer.count())<RXSTRIPE)
            {
              msleep((250*RXSTRIPE)/rxClock);
              if(!soundIOPtr->isCapturing())
                {
                  switchRxState(RXINIT);
                }
            }
          else
            {
              //              addToLog("Load new buf",LOGPERFORM);
              rxBytes+=RXSTRIPE;
              //              addToLog(QString("rxBytes=%1").arg(rxBytes),LOGRXFUNC);
              soundIOPtr->rxBuffer.copyNoCheck(tempBuf,RXSTRIPE);
              soundIOPtr->rxVolumeBuffer.copyNoCheck(volBuf,RXSTRIPE);
              displayFFTEvent* ce = new displayFFTEvent(tempBuf);
              QApplication::postEvent(dispatcherPtr, ce);

              addToLog("fft display done",LOGPERFORM);
              switch (transmissionModeIndex)
                {
                case TRXDRM:
                  addToLog("drmRxPtr->run",LOGPERFORM);
                  drmRxPtr->run(tempBuf);
                  break;
                case TRXSSTV:
                  sstvRxPtr->run(tempBuf,volBuf);
                  break;
                case TRXNOMODE:
                  switchRxState(RXIDLE);
                  break;
                }
            }
          break;
        case RXINIT:
          forceInit();
          switchRxState(RXIDLE);
          break;
        case RXRESTART:
          {
            init();
            switchRxState(RXRUNNING);
          }
          break;
        }
    }
  abort=false;
  rxState=RXIDLE;

}

void rxFunctions::stopThread()
{
  abort=true;
  if(!isRunning()) return;
  while(abort)
    {
      qApp->processEvents();
    }
}

void rxFunctions::init()
{
  switchRxState(RXINIT);
}

void rxFunctions::forceInit()
{

  if(transmissionModeIndex==TRXDRM)
    {

      drmRxPtr->init();
    }
  else
    {
      sstvRxPtr->init();
    }
}

bool rxFunctions::rxBusy()
{
  switch (transmissionModeIndex)
    {
    case TRXDRM:
      return drmBusy;
      break;
    case TRXSSTV:
      return sstvRxPtr->isBusy();
      break;
    case TRXNOMODE:
       return false;
      break;
    }
  return false;
}


void rxFunctions::stopAndWait()
{
  if(soundIOPtr) soundIOPtr->idleRX();
  switchRxState(RXINIT);
  if(!isRunning())
    {
      return;
    }
  while((rxState!=RXIDLE) && (isRunning()))
    {
      qApp->processEvents();
    }
}

void rxFunctions::restartRX()
{
  switchRxState(RXRESTART);
}

void rxFunctions::startRX()
{
  switchRxState(RXRUNNING);
}

void rxFunctions::eraseImage()
{
  if(isRunning())
    {
      if(transmissionModeIndex==TRXDRM)
        {
          drmRxPtr->eraseImage();
        }
      else
        {
          sstvRxPtr->eraseImage();
        }
    }
}

void rxFunctions::switchRxState(erxState newState)
{
  addToLog(QString("%1 to %2").arg(rxStateStr[rxState]).arg(rxStateStr[newState]),LOGRXFUNC);
  rxState=newState;
}




#ifndef QT_NO_DEBUG
unsigned int rxFunctions::setOffset(unsigned int offset,bool ask)
{
  return sstvRxPtr->setOffset(offset,ask);
}
#endif
