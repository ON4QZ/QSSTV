#ifndef DRMTX_H
#define DRMTX_H


#include <QObject>
#include <QFileInfo>
#include <QTemporaryFile>


#include "hybridcrypt.h"
#include "drmtransmitter.h"
#include "reedsolomoncoder.h"
#include "ftpthread.h"
#include "ftpfunctions.h"

class imageViewer;

struct txSession
{
  drmTxParams drmParams;
  QByteArray ba;  // contains the image data in jpg, jp2 .... format
  uint transportID;
  QString filename;
  QString extension;
};



class drmTransmitter;

class drmTx : public QObject
{
  Q_OBJECT
public:
  explicit drmTx(QObject *parent = 0);
  ~drmTx();
  void init();
  void start();
  void forgetTxFileName();
  QString getTxFileName(QString fileName);
  bool ftpDRMHybrid(QString fileName, QString destName);
  bool initDRMImage(bool binary, QString fileName);
  void updateTxList();
  void who();
  void sendBSR(QByteArray *p,drmTxParams dp);
  int processFIX(QByteArray bsrByteArray);
  void initDRMBSR(QByteArray *ba);
  bool initDRMFIX(txSession *sessionPtr);
//  bool initDRMFIX(QString fileName,QString extension,eRSType rsType,int mode);
  txSession *getSessionPtr(uint transportID);
  void applyTemplate(QString templateFilename, bool useTemplate, imageViewer *ivPtr);


  void setTxParams(drmTxParams params)
  {
    drmTxParameters=params;
  }
 double calcTxTime(int overheadTime);
 void clearLastHybridUpload();


private slots:
//  void rxNotification(QString info);

private:
    void runRx();
    void ftpDRMHybridNotifyCheck(QString mask);
    drmTransmitter *txDRM;
    QList <txSession> txList;
    drmTxParams drmTxParameters;
    drmTxParams drmTxHybridParameters;
    QFileInfo   drmTxFileName;
    QString	drmTxStamp;
    int		hybridTxCount;
    QByteArray baDRM;
    QString lastHybridUpload;
    QTemporaryFile ftmp;
    ftpFunctions ff;
};

#endif // DRMTX_H
