#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "dispatchevents.h"
#include "appglobal.h"
#include <QByteArray>
#include "textdisplay.h"
#include "txfunctions.h"
#include "ftpfunctions.h"
class editor;
class imageViewer;


#include <QProgressDialog>

enum eftpResult {DFTPWAITING,DFTPOK,DFTPERROR};

/**
@author Johan Maes
*/
class dispatcher : public QObject
{
  Q_OBJECT

public:

  dispatcher();
  ~dispatcher();
  void init();
  void idleAll();
  void startRX();
  void startTX(txFunctions::etxState state);
  void readSettings();
  void writeSettings();
  void customEvent( QEvent * e );
  void startDRMFIXTx(QByteArray ba);
  void startDRMTxBinary();
  //  void startDRMHybridTx(QString fn);
  //  void startDRMHybridText(QString txt);
  //  void sendSweepTone(double duration,double lowerFreq,double upperFreq);
  void saveImage(QString fileName, QString infotext);
  void uploadToRXServer(QString remoteDir, QString fn);
  void logSSTV(QString call, bool fromFSKID);
  void showOffLine();
//  eftpResult notifyRXDone;
//  eftpResult hybridTxDone;
//  eftpResult hybridRxDone;
//  eftpResult notifyTxDone;


private slots:
//  void slotRXNotification(QString info);
  void slotTXNotification(QString);
  void slotRenameListing(bool err);

private:
  void saveRxSSTVImage(esstvMode mode);
  bool inList(QList <QUrlInfo> lst,QString fn);
  void timerEvent(QTimerEvent *event);
  bool editorActive;
  editor *ed;
  imageViewer *iv;
  int txTimeCounter;
  int prTimerIndex;
  int logTimerIndex;
  textDisplay *infoTextPtr;
  QMessageBox *mbox;
  QProgressDialog *progressFTP;
  QString lastFileName;
  QString lastCallsign;
  QDateTime saveTimeStamp;
  ftpFunctions ff;
  QString uploadSourceFile;
};
#endif

