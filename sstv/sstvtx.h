#ifndef SSTVTX_H
#define SSTVTX_H
#include "sstvparam.h"
#include "testpatternselection.h"


class modeBase;
class imageViewer;

class sstvTx
{
public:
  sstvTx();
  ~sstvTx();
  void init();
  double calcTxTime(int overheadTime);
  bool sendImage(imageViewer *ivPtr);
  void abort();
  bool aborted();
  void applyTemplate(QString templateFilename, bool useTemplate, imageViewer *ivPtr);
  void createTestPattern(imageViewer *ivPtr, etpSelect sel);


private:
  modeBase *currentMode;
  bool create(esstvMode m,DSPFLOAT clock);
  void sendPreamble();
  void sendVIS();
  esstvMode oldMode;
  unsigned long sampleCounter;
  double FSKIDTime();



};

#endif // SSTVTX_H
