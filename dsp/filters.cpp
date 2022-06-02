#include "filters.h"
#include "filter.h"
#include "appglobal.h"

#include <QDebug>



syncFilter:: syncFilter(uint maxLength):
  sync1200(filter::FTIIR,maxLength),sync1900(filter::FTIIR,maxLength),
  sync1200lp(filter::FTFIR,maxLength),sync1900lp(filter::FTFIR,maxLength)
{
  init();
}

syncFilter::~syncFilter()
{

}

void syncFilter:: init()
{
  // setup the syncFilters
  sync1200.init();
  sync1200.nZeroes=SYNCBPNUMZEROES;
  sync1200.nPoles=SYNCBPNUMPOLES;
  sync1200.gain=SYNCBP1200GAIN;
  sync1200.coefZPtr=(FILTERPARAMTYPE *)z_sync_bp1200;
  sync1200.coefPPtr=(FILTERPARAMTYPE *)p_sync_bp1200;
  sync1200.allocate();

  sync1900.init();
  sync1900.nZeroes=SYNCBPNUMZEROES;
  sync1900.nPoles=SYNCBPNUMPOLES;
  sync1900.gain=SYNCBP1900GAIN;
  sync1900.coefZPtr=(FILTERPARAMTYPE *)z_sync_bp1900;
  sync1900.coefPPtr=(FILTERPARAMTYPE *)p_sync_bp1900;
  sync1900.allocate();

  sync1200lp.init();
  sync1200lp.nZeroes=SYNCLPTAPS;
  sync1200lp.gain=SYNCLPGAIN;
  sync1200lp.coefZPtr=(FILTERPARAMTYPE *)z_sync_lp;
  sync1200lp.allocate();

  sync1900lp.init();
  sync1900lp.nZeroes=SYNCLPTAPS;
  sync1900lp.gain=SYNCLPGAIN;
  sync1900lp.coefZPtr=(FILTERPARAMTYPE *)z_sync_lp;
  sync1900lp.allocate();

  detect1200Ptr= sync1200lp.filteredPtr;
  detect1900Ptr= sync1900lp.filteredPtr;
}

void syncFilter::process(FILTERPARAMTYPE *dataPtr)
{
  sync1200.processIIRRectified(dataPtr);
  sync1200lp.processFIR(sync1200.filteredPtr,sync1200lp.filteredPtr);
#ifndef DISABLENARROW
  sync1900.processIIRRectified(dataPtr);
  sync1900lp.processFIR(sync1900.filteredPtr,sync1900lp.filteredPtr);
#endif
}






videoFilter::videoFilter(uint maxLength):videoFltr(filter::FTFIR,maxLength),lpFltr(filter::FTFIR,maxLength)
{
  init();
}

videoFilter::~videoFilter()
{
}

void videoFilter::init()
{
  videoFltr.init();
  lpFltr.init();
  videoFltr.volumeAttackIntegrator=0.07;
  videoFltr.volumeDecayIntegrator=0.01;
  videoFltr.nZeroes=VIDEOFIRNUMTAPS-1;
  videoFltr.gain=VIDEOFIRGAIN;
  videoFltr.frCenter=VIDEOFIRCENTER;
  videoFltr.coefZPtr=(FILTERPARAMTYPE *)videoFilterCoefFIR;
  videoFltr.allocate();
  demodPtr=videoFltr.demodPtr;
  lpFltr.setupMatchedFilter(0,1);
}

void videoFilter::process(FILTERPARAMTYPE *dataPtr)
{

  videoFltr.processFIRDemod(dataPtr,videoFltr.filteredPtr);
  lpFltr.processFIRInt(videoFltr.filteredPtr,videoFltr.demodPtr);
}



wfFilter::wfFilter(uint maxLength):wfFltr(filter::FTFIR,maxLength)
{
  init();
}

wfFilter::~wfFilter()
{

}

void wfFilter::init()
{
  wfFltr.init();
  wfFltr.nZeroes=TXWFNUMTAPS-1;
  wfFltr.gain=1;
  wfFltr.coefZPtr=(FILTERPARAMTYPE *)wfFilterCoef;
  wfFltr.volumeAttackIntegrator=0.07;
  wfFltr.volumeDecayIntegrator=0.01;
  wfFltr.allocate();
}

void  wfFilter::process(double *dataPtr, uint dataLength)
{
  wfFltr.dataLen=dataLength;
  wfFltr.processFIR(dataPtr,dataPtr);
}



drmHilbertFilter::drmHilbertFilter(uint maxLength):drmFltr(filter::FTFIR,maxLength)
{
  init();
}

drmHilbertFilter::~drmHilbertFilter()
{
}

void drmHilbertFilter::init()
{
  drmFltr.init();
  drmFltr.nZeroes=DRMHILBERTTAPS-1;
  drmFltr.nPoles=0;
  drmFltr.coefZPtr=(FILTERPARAMTYPE *)drmHilbertCoef;
  drmFltr.gain=DRMHILBERTGAIN;
  drmFltr.allocate();
}

//void drmHilbertFilter::process(float *dataPtr,uint dataLength)
//{

//  process(dataPtr,dataPtr,dataLength);
//}

void drmHilbertFilter::process(FILTERPARAMTYPE *dataPtr, float *outputPtr,uint dataLength)
{
  drmFltr.dataLen=dataLength;
  drmFltr.processIQ(dataPtr,outputPtr);
}



