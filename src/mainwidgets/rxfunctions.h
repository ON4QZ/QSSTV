#ifndef RXFUNCTIONS_H
#define RXFUNCTIONS_H

#include <QThread>
#include "appdefs.h"
#include "sstv/syncprocessor.h"
#include "buffermanag.h"




class downsampleFilter;
class iirFilter;
class modeBase;
class sstvRx;
class drmRx;

class rxFunctions : public QThread
{

  Q_OBJECT
public:
  enum erxState {RXIDLE,RXRUNNING,RXRESTART,RXINIT};
  explicit rxFunctions(QObject *parent = 0);
  ~rxFunctions();
  void run();
  void init();
  void stopAndWait();
  void startRX();
  void restartRX();
  void eraseImage();
  QString getModeStr();
  sstvRx  *sstvRxPtr;
  void stopThread();
  bool rxBusy();

#ifdef ENABLESCOPE
  unsigned int setScopeParam(unsigned int offset,unsigned int numSamples,bool ask);
#endif
private:

  drmRx *drmRxPtr;
  bool abort;
  erxState rxState;
  void switchRxState(erxState newState);
  uint rxBytes;
  void forceInit();

};

#endif // RXFUNCTIONS_H
