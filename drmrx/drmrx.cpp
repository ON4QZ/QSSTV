#include "drmrx.h"
#include "appglobal.h"
#include "drm.h"
#include "demodulator.h"
#include "dispatcher.h"
#include "filters.h"


drmRx::drmRx(QObject *parent) : QObject(parent),iqFilter(RXSTRIPE)
{
  srcDecoder=new sourceDecoder;
  demodulatorPtr=new demodulator;
}

drmRx::~drmRx()
{
  delete srcDecoder;
}

void drmRx::init()
{
  avgSNR=0;
  avgSNRAvailable=false;
  n = DRMBUFSIZE;
  /* initialisations */
  demodulatorPtr->init();
  initGetmode( n / 4);
  rRation = 1.000;
  samplerate_offset_estimation = 0.0;
  runstate = RUN_STATE_POWER_ON;		/* POWER_ON */
  channel_decoding();
  runstate = RUN_STATE_INIT;		/* INIT */
  channel_decoding();
  runstate = RUN_STATE_FIRST;			/* FIRSTRUN */
  runstate = RUN_STATE_NORMAL;			/* NORMAL RUN */
  srcDecoder->init();
}



void drmRx::run(DSPFLOAT *dataPtr)
{
  bool done=false;
  DSPFLOAT temp;
  displayDRMStatEvent *ce1;
  displayDRMInfoEvent *ce2 ;

  temp=WMERFAC;
  if(temp<0) temp=0;
  if(avgSNRAvailable)
    {
      avgSNR=(1-0.05)*avgSNR+0.05*temp;
      ce1 = new displayDRMStatEvent(avgSNR);
      ce1->waitFor(&done);
      QApplication::postEvent(dispatcherPtr, ce1);
      while(!done) { usleep(10);}

    }

 if(input_samples_buffer_request ==0)
    {
      demodulatorPtr->demodulate(resamp_signal,0);
    }
  iqFilter.process(dataPtr,resamp_signal,RXSTRIPE);
  im=RXSTRIPE;

  demodulatorPtr->demodulate(resamp_signal,im);

  done=false;
  ce2 = new displayDRMInfoEvent;
  ce2->waitFor(&done);
  QApplication::postEvent(dispatcherPtr, ce2);
  while(!done) { usleep(10);}
}

