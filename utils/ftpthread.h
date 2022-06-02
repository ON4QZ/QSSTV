#ifndef FTPTHREAD_H
#define FTPTHREAD_H

#include "ftpevents.h"

#include <QObject>
#include <QEvent>
#include <QTimer>
#include <QThread>

#include "qftp.h"

#define FTPTIMEOUTTIME 12000

class QFtp;
class QFile;

enum eftpError {FTPOK,FTPERROR,FTPTIMEOUT};

class ftpThread : public QObject
{
  Q_OBJECT
public:
  explicit ftpThread(QString id);
  ~ftpThread();

  void setHostParams(QString tHost, int tPort, QString tUser, QString tPasswd, QString tDirectory);
public slots:
//  void slotStart();
  void customEvent( QEvent * ev );
  QList <QUrlInfo> getList()
  {
    return listingResults;
  }
  void disconnectFtp();

private slots:
  void slotInit();
  void slotTimeout();
  void slotDisconnect();
  void slotCommandStarted(int);
  void slotCommandFinished(int,bool);
  void slotDone(bool);
  void slotStateChanged(int state);
  void slotListInfo(const QUrlInfo &);
  void slotRawCommandReply(int, const QString &);
  void slotProgress(qint64,qint64);

signals:
  void finished();
  void commandsDone(int error,QString errorStr);
  void listingComplete(bool err);
  void downloadFinished(bool err,QString fn);
  void ftpQuit();

private:
  QString idName;
  bool canCloseWhenDone;
  bool displayProgress;
  QFtp *qftpPtr;

  void setupConnection(QString tHost,int tPort,QString tUser,QString tPasswd,QString tDirectory);
  void destroy();
  void doConnect();
  void connectToHost();
  void changePath( const QString &newPath );
  int listFiles(QString mask);
  void uploadFile(QString source, QString destination);
  void downloadFile(QString source, QString destination);
  void rename(QString source, QString destination);
  void remove(QString source);
  void wait(int timeout);

  QString host;
  int port;
  QString user;
  QString passwd;
  QString directory;
  QString source;
  QString destination;

  bool ftpDone;
  bool connectPending;
  bool ftpCommandSuccess;
  bool aborting;
  bool stopThread;

  QTimer *timeoutTimerPtr;
  QTimer *disconnectTimerPtr;
  QTimer *notifyTimerPtr;

  bool   timeoutExpired;
  int timeoutValue;

//  void slotStateChanged( int );
  void dumpState( int state );


  bool isLoggedIn();
  bool isUnconnected();
  bool isBusy();


//  void execFTPTest();

  eftpError lastError;
  QString lastErrorStr;
  QFile *sourceFn;
  QFile *destFn;
  QList <QUrlInfo> listingResults;
  bool removeFiles;
};

#endif // FTPTHREAD_H
