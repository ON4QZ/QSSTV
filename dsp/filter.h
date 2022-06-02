#ifndef FILTER_H
#define FILTER_H
#include "filterparam.h"
#include "nco.h"

class filter
{

public:
  enum efilterType {FTNONE,FTFIR,FTIIR,FTHILB};
  filter(efilterType fType,uint dataLenght);
  ~filter();
  void process();
  void init();
  void processFIR(FILTERPARAMTYPE *dataPtr,FILTERPARAMTYPE *dataOutputPtr);
  void processFIRInt(FILTERPARAMTYPE *dataPtr, quint16 *dataOutputPtr);
  void processFIRDemod(FILTERPARAMTYPE *dataPtr, FILTERPARAMTYPE *dataOutputPtr);
  void processIIRRectified(FILTERPARAMTYPE *dataPtr);
  void processHILBVolume(FILTERPARAMTYPE *dataPtr);
  void processIQ(FILTERPARAMTYPE *dataPtr,float *dataOutputPtr);
  void setupMatchedFilter(FILTERPARAMTYPE freq, uint numTaps);
  void allocate();
  FILTERPARAMTYPE *coefZPtr;
  FILTERPARAMTYPE *coefPPtr;
  FILTERPARAMTYPE *filteredPtr;
  FILTERPARAMTYPE *volumePtr;
  FILTERPARAMTYPE gain;
  quint16 *demodPtr;
  FILTERPARAMTYPE frCenter;
  FILTERPARAMTYPE volumeAttackIntegrator;
  FILTERPARAMTYPE volumeDecayIntegrator;
  uint nZeroes;
  uint nPoles;
  uint dataLen;
  int fltrIndex;
  unsigned int coefIndex;
  bool coefZPtrNewed;

private:

  void resetPointers();
  void deleteBuffers();

  FILTERPARAMTYPE *sampleBufferIPtr;
  FILTERPARAMTYPE *sampleBufferQPtr;
  FILTERPARAMTYPE *sampleBufferYIPtr;



  FILTERPARAMTYPE resIprev;
  FILTERPARAMTYPE resQprev;

  FILTERPARAMTYPE angleToFc;
//  FILTERPARAMTYPE avgVolume;
  FILTERPARAMTYPE prevTemp;

  NCO nco;


  uint bufSize;
  efilterType filterType;
};




#endif // FILTER_H
