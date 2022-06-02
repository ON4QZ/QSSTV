#ifndef TXFUNCTIONS_H
#define TXFUNCTIONS_H

#include <QThread>
#include "appdefs.h"
#include "drmparams.h"
#include "testpatternselection.h"

#define SILENCEDELAY 0.600           // send silence after transmission

extern int templateIndex;
extern bool useTemplate;
extern bool useCW;
extern bool useVOX;

class sstvTx;
class drmTx;
class imageViewer;



class txFunctions  : public QThread
{
  Q_OBJECT
public:
  enum etxState
  {
    TXIDLE,	//!< in idle loop
    TXACTIVE,
    TXSENDTONE,
    TXSENDWFID,
    TXSENDCWID,
    TXSENDDRM,
    TXSENDDRMPIC,
    TXSENDDRMBINARY,
    TXSENDDRMBSR,
    TXSENDDRMFIX,
    TXSENDDRMTXT,
    TXSSTVIMAGE,
    TXSSTVPOST,
    TXRESTART,
    TXPREPARESSTV,
    TXPREPAREDRMPIC,
    TXPREPAREDRMBINARY,
    TXTEST

  };
  txFunctions(QObject *parent);
  ~txFunctions();
  void init();
  void run();
  void stopThread();
  void startTX(etxState state);
  void prepareTX(etxState state);
  void stopAndWait();
  void setToneParam(double duration,double lowerFreq,double upperFreq=0)
  {
    toneDuration=duration;
    toneLowerFrequency=lowerFreq;
    toneUpperFrequency=upperFreq;
  }
  bool prepareFIX(QByteArray bsrByteArray);
  bool prepareBinary(QString fileName);
  void sendBSR(QByteArray *p,drmTxParams dp);
  void applyTemplate(imageViewer *ivPtr,QString templateFilename);
  void forgetTxFileName();
  etxState getTXState() { return txState;}
  void setDRMTxParams(drmTxParams params);
//  void setOnlineStatus(bool online, QString info="");
  //  bool initDRMFIX(txSession *sessionPtr);
  int calcTxTime(bool binary, int overhead);
  void txTestPattern(imageViewer *ivPtr, etpSelect sel);
  bool txBusy();

private:
  void waitTxOn();
  void waitEnd();
  void sendCW();
  void sendFSKID();
  void sendTestPattern();
  void syncBurst();
  void sendFSKChar(int IDChar);
  void switchTxState(etxState newState);
  void startProgress(double duration);
  void prepareTXComplete(bool ok);
  etxState txState;
  bool started;
  bool abort;
  double toneDuration;
  double toneLowerFrequency;
  double toneUpperFrequency;
  QString binaryFilename;
  sstvTx *sstvTxPtr;
  drmTx *drmTxPtr;
};

#endif // TXFUNCTIONS_H
