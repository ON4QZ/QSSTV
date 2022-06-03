#ifndef FTPFUNCTIONS_H
#define FTPFUNCTIONS_H

#include <QString>
#include <QObject>
#include <QMessageBox>
#include <QTemporaryFile>
#include "ftpthread.h"

class ftpThread;

class ftpFunctions: public QObject
{
  Q_OBJECT
public:
  ftpFunctions();
  ~ftpFunctions();
  bool test(QString name, QString tHost, int tPort, QString tUser, QString tPasswd, QString tDirectory, bool doSetup);
  void uploadFile(QString source, QString destination, bool wait, bool closeWhenDone);
  void uploadData(QByteArray ba, QString destination, bool wait, bool closeWhenDone);
  void downloadFile(QString source, QString destination, bool wait, bool closeWhenDone);
  void remove(QString source, bool wait, bool closeWhenDone);
  void rename(QString tSource,QString tDestination,bool wait,bool closeDone);
  void setupFtp(QString name, QString tHost, int tPort, QString tUser, QString tPasswd, QString tDirectory);
  void listFiles(QString mask, bool closeWhenDone);
  void mremove(QString mask,bool wait,bool closeWhenDone);
  void disconnectFtp();
  eftpError getLastError() {return lastError;}
  eftpError getLastErrorStr(QString &lastErrorString)
  {
    lastErrorString=lastErrorStr;
    return lastError;}
  QList<QUrlInfo> getListing();
  bool isBusy() {return busy;}
  void changePath(QString source, bool wait);
  void changeThreadName(QString tidName);


private slots:
  void slotCommandsDone(int err, QString errStr);
  void slotThreadFinished();
  void slotDownloadFinished(bool err,QString filename);
  void slotListingFinished(bool err);


signals:
  void downloadDone(bool,QString);
  void listingDone(bool err);
private:
  void setupFtp();

  bool checkUpload(QString fn, QString rfn);
  bool checkRemove(QString rfn);
  bool checkStart();
  void checkWait(bool wait);

  ftpThread *ftpThr;
  QString idName;
  QString host;
  int port;
  QString user;
  QString passwd;
  QString directory;

  QMessageBox mb;
  bool endOfCommands;
  eftpError lastError;
  QString lastErrorStr;
  QThread* thread;
  QTemporaryFile ftmp;
  int commandID;
  bool busy;
  bool mremoveCmd;


};

#endif // FTPFUNCTIONS_H

