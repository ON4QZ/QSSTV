#ifndef FILTERS_H
#define FILTERS_H
#include "filter.h"


class syncFilter
{
public:
  syncFilter(uint maxLength);
  ~syncFilter();
  void process(FILTERPARAMTYPE *dataPtr);
  void init();
  FILTERPARAMTYPE *detect1200Ptr;
  FILTERPARAMTYPE *detect1900Ptr;
private:
  filter sync1200;
  filter sync1900;
  filter sync1200lp;
  filter sync1900lp;


};


class videoFilter
{
public:
  videoFilter(uint maxLength);
  ~videoFilter();
  void process(FILTERPARAMTYPE *dataPtr);
  void init();
  quint16 *demodPtr;
private:
  filter videoFltr;
  filter lpFltr;
};

class wfFilter
{
public:
  wfFilter(uint maxLength);
  ~wfFilter();
  void process(FILTERPARAMTYPE *dataPtr, uint dataLength=RXSTRIPE);
  void init();
private:
  filter wfFltr;
};

class drmHilbertFilter
{
public:
  drmHilbertFilter(uint maxLength);
  ~drmHilbertFilter();
//  void process(FILTERPARAMTYPE *dataPtr, uint =RXSTRIPE);
  void process(FILTERPARAMTYPE *dataPtr, float *outputPtr,uint dataLength);
  void init();
private:
  filter drmFltr;
};


#endif // FILTERS_H
