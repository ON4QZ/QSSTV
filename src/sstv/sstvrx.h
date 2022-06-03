#ifndef SSTVRX_H
#define SSTVRX_H

#include <QObject>
#include "appdefs.h"
#include "buffermanag.h"
#include "syncprocessor.h"



extern int minCompletion;



class modeBase;

class syncFilter;
class videoFilter;
class rectConvolution;
class integrator;

class sstvRx : public QObject
{

  Q_OBJECT
public:
  enum eSSTVState {HUNTING,PROCESSING,WAITFORSYNC,SLANTADJUST,RESTART,SYNCLOST,WAIT,END};
  explicit sstvRx(QObject *parent = 0);
  ~sstvRx();
  void init();
  void setFilters();
  void run(DSPFLOAT *dataPtr, DSPFLOAT *volumePtr);
  void eraseImage();
  syncProcessor syncNarrowProc;
  syncProcessor   syncWideProc;
  bool isBusy()
  {
    return((SSTVState!=HUNTING) && (SSTVState!=END));
  }
#ifndef QT_NO_DEBUG
  unsigned int setOffset(unsigned int offset,bool ask);
#endif

signals:
  void resetCall();
private:
  //  void getData();
  void process();
  void switchState(eSSTVState  newState);
  void resetParams(bool bufferReset);
  void saveImage();
  void advanceBuffers();
  void rewindBuffers(uint rlen);


  //  DSPFLOAT rxHoldingBuffer[RXSTRIPE];
  buffer<quint16,22> bufferVideoDemod;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
  //  buffer<quint16,22> bufferSyncDemod;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
  buffer<DSPFLOAT,22> bufferSync1200Vol;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
  buffer<DSPFLOAT,22> bufferSync1900Vol;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
  buffer<DSPFLOAT,22> bufferTest;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
  buffer<DSPFLOAT,22> bufferInputVol;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
//  buffer<unsigned int,22> bufferInputVol;  // 2^22= 4194304, divided by samplingrate 12000 gives 349 seconds buffering
  //  buffer<quint16,22> bufferInputVolume;

  syncFilter *syncFilterPtr;
  videoFilter *videoFilterPtr;
  DSPFLOAT tempBuffer[RXSTRIPE];
  rectConvolution *syncConvolvePtr;
  integrator *volumeIntergratorPtr;
  eSSTVState SSTVState;
  unsigned int bufferCounter;
  QString lastUsedModeStr;
  unsigned int bufferIdx;
  unsigned int currentIdx;
  bool usingWide;
  syncProcessor *syncProcPtr;
  DSPFLOAT agcVolume;
};

#endif // SSTVRX_H
