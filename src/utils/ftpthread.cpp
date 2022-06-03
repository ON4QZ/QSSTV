#include "ftpthread.h"
#include "appglobal.h"
#include "configparams.h"
#include "logging.h"
#include "dispatcher.h"

#include <qglobal.h>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QDebug>



QString commandStr[15]=
{
  "None",
  "SetTxMode",
  "SetProxy",
  "ConnectToHost",
  "Login",
  "Close",
  "List",
  "Cd",
  "Get",
  "Put",
  "Remove",
  "Mkdir",
  "Rmdir",
  "Rename",
  "RawCommand"
};


ftpThread::ftpThread(QString id) : QObject()
{
  qftpPtr=NULL;
  addToLog(QString("FTP Created: %1").arg(id),LOGFTPTHREAD);
  idName=id;
  canCloseWhenDone=false;
  displayProgress=true;
  aborting=false;
  stopThread=false;
  sourceFn=NULL;
  destFn=NULL;
//  qDebug()<< "ftpThread constructor";
}

ftpThread::~ftpThread()
{
//  qDebug()<< "ftpThread destructor";
}

void ftpThread::slotInit()
{
  timeoutTimerPtr=new QTimer(this);
  disconnectTimerPtr=new QTimer(this);
  notifyTimerPtr=new QTimer(this);
  destroy();
  if(qftpPtr) delete qftpPtr;
  addToLog(QString("FTP init '%1'").arg(idName), LOGFTPTHREAD);

  qftpPtr = new QFtp;

  disconnectTimerPtr->setInterval(FTPTIMEOUTTIME);

  connect( qftpPtr, SIGNAL(commandStarted(int)),SLOT(slotCommandStarted(int)) );
  connect( qftpPtr, SIGNAL(commandFinished(int,bool)),SLOT(slotCommandFinished(int,bool)) );
  connect( qftpPtr, SIGNAL(done(bool)),SLOT(slotDone(bool)) );
  connect( qftpPtr, SIGNAL(stateChanged(int)),SLOT(slotStateChanged(int)) );
  connect( qftpPtr, SIGNAL(listInfo(const QUrlInfo &)),SLOT(slotListInfo(const QUrlInfo &)) );
  connect( qftpPtr, SIGNAL(rawCommandReply(int, const QString &)),SLOT(slotRawCommandReply(int, const QString &)) );
  connect( qftpPtr, SIGNAL(dataTransferProgress(qint64,qint64)),SLOT(slotProgress(qint64,qint64)) );
//  connect(notifyTimerPtr,     SIGNAL(timeout()), this, SLOT(notifyTick()));
  connect(timeoutTimerPtr,    SIGNAL(timeout()), this, SLOT(slotTimeout()));
  connect(disconnectTimerPtr, SIGNAL(timeout()), this, SLOT(slotDisconnect()));

  timeoutTimerPtr->setSingleShot(true);
  timeoutTimerPtr->setInterval(FTPTIMEOUTTIME);
  lastError=FTPOK;
  lastErrorStr="";
  disconnectTimerPtr->setInterval(2000);
}

void ftpThread::setHostParams(QString tHost, int tPort, QString tUser, QString tPasswd, QString tDirectory)
{
  host=tHost;
  port=tPort;
  user=tUser;
  passwd=tPasswd;
  directory=tDirectory;
}

void ftpThread::disconnectFtp()
{
  if (!isUnconnected())
   {
    addToLog(QString("'%1': schedule disconnect in %2ms").arg(idName).arg(disconnectTimerPtr->interval()),LOGFTPTHREAD);
    disconnectTimerPtr->start();
  }
}



void ftpThread::customEvent( QEvent * e)
{
  QString source;
  QString destination;
  ftpEventType type;
  type=(ftpEventType)e->type();
  addToLog(QString("customevent:%1 in thread: %2").arg(((ftpBaseEvent *)e)->description).arg(QThread::currentThread()->objectName()),LOGFTPTHREAD);
  source=((ftpBaseEvent *)e)->source;
  destination=((ftpBaseEvent *)e)->destination;
  canCloseWhenDone=((ftpBaseEvent *)e)->closeWhenDone;

  switch (type)
    {
    case ftp_list:
      addToLog(QString("customevent ftp_list %1").arg(source),LOGFTPTHREAD);
      listingResults.clear();
      listFiles(source);
    break;
    case ftp_remove:
      addToLog(QString("customevent ftp_remove %1").arg(source),LOGFTPTHREAD);
      remove(source);
      break;
    case ftp_rename:
      addToLog(QString("customevent ftp_rename %1 to %2").arg(source).arg(destination),LOGFTPTHREAD);
      rename(source,destination);
      break;
    case ftp_upload:
      addToLog(QString("customevent ftp_upload %1 to %2").arg(source).arg(destination),LOGFTPTHREAD);
      uploadFile(source,destination);
      break;
    case ftp_download:
      addToLog(QString("customevent ftp_download %1 to %2").arg(source).arg(destination),LOGFTPTHREAD);
      downloadFile(source,destination);
      break;
    case ftp_disconnect:
      addToLog(QString("customevent ftp_disconnect"),LOGFTPTHREAD);
      disconnectFtp();
      break;
     case ftp_cd:
      doConnect();
      qftpPtr->cd(source);
      break;
    default:
      addToLog(QString("unsupported event: %1").arg(((ftpBaseEvent*)e)->description), LOGALL);
      break;
    }
}


void ftpThread::setupConnection(QString tHost,int tPort,QString tUser,QString tPasswd,QString tDirectory)
{
  addToLog(QString("'%1' host: %2, User: %3,directory: %4").arg(idName).arg(tHost).arg(tUser).arg(tDirectory), LOGFTPTHREAD);
  if(host!=tHost || user!=tUser || passwd!=tPasswd || port!=tPort)
    {
      if (qftpPtr) destroy();
    }
  host=tHost;
  port=tPort;
  user=tUser;
  passwd=tPasswd;
  directory=tDirectory;
  changePath(tDirectory);
}


void ftpThread::destroy()
{
  disconnectTimerPtr->stop();
  //  mremove_listids.clear();
  //  listingResults.clear();
  //  notifyId=-1;
  ftpDone=true;
  connectPending=false;
  ftpCommandSuccess=false;
  if(qftpPtr)
    {
      addToLog("FTP show state in destroy",LOGFTPTHREAD);
      slotStateChanged(qftpPtr->state());
      if( qftpPtr->state() != QFtp::Unconnected )
        {
          addToLog(QString("Closing '%1' to %2").arg(idName).arg(host),LOGFTPTHREAD);
          qftpPtr->close();
        }
    }

}

void ftpThread::doConnect()
{
  aborting=false;
  if(isUnconnected() && !connectPending)
    {
      //addToLog(QString("FTP connect to host %1").arg(host),LOGFTPTHREAD);
      connectToHost();
    }
  else if (connectPending)
    {
      addToLog(QString("'%1' connection pending to %2").arg(idName).arg(host),LOGFTPTHREAD);
    }
  else
    {
//      addToLog(QString("'%1' already connected to %2, %3").arg(idName).arg(host).arg(commandStr[qftpPtr->currentCommand()]),LOGFTPTHREAD);
    }
}

void ftpThread::connectToHost()
{
  int id;
  Q_UNUSED (id);
  addToLog(QString("'%1' connectToHost %2").arg(idName).arg(host), LOGFTPTHREAD);
  destroy();
  ftpDone=false;
  connectPending=true;
  qftpPtr->connectToHost(host,port);
  qftpPtr->login( user, passwd );

  if(!directory.isEmpty())
    {
      id = qftpPtr->cd(directory);
      addToLog(QString("'%1': cd '%2' id:%3").arg(idName).arg(directory).arg(id),LOGFTPTHREAD);
    }
}

void ftpThread::changePath( const QString &newPath )
{
  int id;
  Q_UNUSED (id);
  if ((directory != newPath) || (isUnconnected()))
    {
      directory="";
      doConnect();
      id = qftpPtr->cd( newPath );
      addToLog(QString("'%1':'%2' id:%3").arg(idName).arg(newPath).arg(id), LOGFTPTHREAD);
      directory=newPath;
    }
}

int ftpThread::listFiles(QString mask)
{
  int id;
  doConnect();
  id=qftpPtr->list(mask);
  return id;
}

void ftpThread::uploadFile(QString source,QString destination)
{
  int id;
  Q_UNUSED(id);
  addToLog("uploadFile",LOGFTPTHREAD);
  if (source.isEmpty() )
    {
      addToLog("fileName is empty",LOGFTPTHREAD);
      lastError=FTPERROR;
      lastErrorStr="Filename is empty";
      destroy();
      emit commandsDone((int)lastError,lastErrorStr);
      return;
    }
  sourceFn=new QFile(source);
  if ( !sourceFn->open( QIODevice::ReadOnly ) )
    {
      addToLog(QString("Unable to read '%1'").arg(source), LOGFTPTHREAD);
      lastError=FTPERROR;
      lastErrorStr=QString("Unable to read '%1'").arg(source);
      destroy();
      emit commandsDone((int)lastError,lastErrorStr);
      return;
    }
  QFileInfo fi( source );
  QFileInfo fin(destination);
  addToLog(QString("'%1' put '%2', %3 bytes").arg(idName).arg(destination).arg(sourceFn->size()),LOGFTPTHREAD);
  ftpDone=false;
  doConnect();
  if(fin.fileName().isEmpty()) // no target specified, use the same name as the source
    {
      id=qftpPtr->put( sourceFn, fi.fileName(),QFtp::Binary);
    }
  else
    {
      id=qftpPtr->put( sourceFn, fin.fileName(),QFtp::Binary);
    }
  addToLog(QString("'%1' put '%2', %3 bytes. id=%4").arg(idName).arg(destination).arg(sourceFn->size()).arg(id),LOGFTPTHREAD);
}



void ftpThread::downloadFile(QString source,QString destination)
{
  addToLog("FTP downloadFile",LOGFTPTHREAD);
  if(destFn) delete destFn;
  destFn=new QFile(destination);
  if(!destFn->open(QIODevice::WriteOnly))
    {
      addToLog(QString("Can't open %1 for writing").arg(destination),LOGFTPTHREAD);
      lastError=FTPERROR;
      lastErrorStr=QString("Can't open %1 for writing").arg(destination);
      destroy();
      emit commandsDone((int)lastError,lastErrorStr);
      return;
    }
  if (source.isNull() )
    {
      addToLog("Invalid source",LOGFTPTHREAD);
      lastError=FTPERROR;
      lastErrorStr="Invalid source";
      destroy();
      emit commandsDone((int)lastError,lastErrorStr);
      return;
    }
  ftpDone=false;
  doConnect();
  addToLog(QString("'%1' get '%2' destination '%3'").arg(idName).arg(source).arg(destination),LOGFTPTHREAD);
  qftpPtr->get( source, destFn,QFtp::Binary);
  addToLog(QString("FTP file: %1 bytes: %2").arg(destination).arg(destFn->size()),LOGFTPTHREAD);
}


void ftpThread::remove(QString source)
{
  int id;
  Q_UNUSED(id);
  ftpDone=false;
  doConnect();
  id = qftpPtr->remove(source);
  addToLog(QString("Name=%1 source=%2  id:%3").arg(idName).arg(source).arg(id),LOGFTPTHREAD);
}

void ftpThread::rename(QString source,QString destination)
{
  int id;
  Q_UNUSED(id);
  ftpDone=false;
  doConnect();
  id = qftpPtr->rename(source,destination);
  addToLog(QString("Name=%1 source=%2 destination=%3 id:%4").arg(idName).arg(source).arg(destination).arg(id),LOGFTPTHREAD);
}




void ftpThread::slotStateChanged( int state )
{
  dumpState(state);
}

void ftpThread::dumpState( int state )
{
  switch (state)
    {
    case QFtp::Unconnected:
      addToLog(QString("FTPss Unconnected name:=%1 :host=%2").arg(idName).arg(host),LOGFTPTHREAD);
      break;
    case QFtp::HostLookup:
      addToLog(QString("FTPss Host lookup name:=%1 :host=%2").arg(idName).arg(host),LOGFTPTHREAD);
      break;
    case QFtp::Connecting:
      addToLog(QString("FTPss Connecting name:=%1 :host=%2").arg(idName).arg(host),LOGFTPTHREAD);
      break;
    case QFtp::Connected:
      addToLog(QString("FTPss Connected name:=%1 :host=%2").arg(idName).arg(host),LOGFTPTHREAD);
      break;
    case QFtp::LoggedIn:
      addToLog(QString("FTPss Logged In name:=%1 :host=%2").arg(idName).arg(host),LOGFTPTHREAD);
      break;
    case QFtp::Closing:
      addToLog(QString("FTPss Closing name:=%1 :host=%2").arg(idName).arg(host),LOGFTPTHREAD);
      break;
    default:
      addToLog(QString("FTPss uknown %1 name:=%2 host=%3").arg(qftpPtr->state()).arg(idName).arg(host),LOGFTPTHREAD);
      break;
    }
}

void ftpThread::wait(int timeout)
{
  timeoutTimerPtr->stop();
  if (timeout < 0) timeoutValue = FTPTIMEOUTTIME * (0-timeout);
  else timeoutValue=timeout;
  if (timeoutValue) timeoutTimerPtr->setInterval(timeoutValue);
  addToLog(QString("'%1' tim.start timeout=%2").arg(idName).arg(timeoutValue),LOGFTPTHREAD);
  timeoutTimerPtr->start();
  timeoutExpired=false;

  while (!ftpDone)
    {
      if(aborting)
        {
          lastError=FTPERROR;
          lastErrorStr="FTP command canceled";
          return;
        }
      if(timeout && timeoutExpired)
        {
          addToLog(QString("'%1' Timeout Expired").arg(idName),LOGALL);
          lastError=FTPTIMEOUT;
          return;
        }
    }
  timeoutTimerPtr->stop();
  if(!ftpCommandSuccess)
    {
      lastError=FTPERROR;
      lastErrorStr=qftpPtr->errorString();
      addToLog(QString("Command Fail %1, %2, %3").arg(idName).arg(lastError).arg(lastErrorStr),LOGALL);
    }
  else
    {
      addToLog(QString("Command Success %1, %2, %3").arg(idName).arg(lastError).arg(lastErrorStr),LOGFTPTHREAD);
    }

}


bool  ftpThread::isLoggedIn()
{
  return qftpPtr->state() == QFtp::LoggedIn;
}

bool ftpThread::isUnconnected()
{
  return qftpPtr->state() == QFtp::Unconnected;
}

bool  ftpThread::isBusy()
{
  return !ftpDone;
}




void ftpThread::slotTimeout()
{
  timeoutExpired=true;
}

void ftpThread::slotDisconnect()
{
  addToLog(QString("'%1'").arg(idName), LOGFTPTHREAD);
  destroy();
  emit ftpQuit();
}


/****************************************************************************
**
** Slots connected to signals of the QFtp class
**
*****************************************************************************/

void ftpThread::slotCommandStarted(int id)
{
  Q_UNUSED(id);
  addToLog(QString("'%1' id:%2, %3").arg(idName).arg(id).arg(commandStr[qftpPtr->currentCommand()]),LOGFTPTHREAD);
  if ( qftpPtr->currentCommand() == QFtp::List )
    {
    }
  //  addToLog(QString("'%1' tim.restart interval=%2").arg(name).arg(timeoutTimer.interval()),LOGFTPTHREAD);
  //  timeoutTimer.start();
}

void ftpThread::slotCommandFinished(int id,bool err)
{
  Q_UNUSED(id);
  QIODevice *p;
  QFtp::Command cmd;
  cmd=qftpPtr->currentCommand();

  slotProgress(0,0);

  addToLog(QString("CommandFinished '%1' id:%2,%3 error:%4").arg(idName).arg(id).arg(commandStr[qftpPtr->currentCommand()]).arg(err),LOGFTPTHREAD);
  switch(cmd)
    {
    case QFtp::Login:
      connectPending=false;
      break;
    case QFtp::List:
         emit listingComplete(err);
    break;
    case QFtp::Get:
      emit downloadFinished(err,destFn->fileName());
      break;
    default:
      break;
    }

  if(err)
    {
      addToLog(QString("FTP error: %1 %2").arg(idName).arg(qftpPtr->errorString()),LOGFTPTHREAD);
      ftpCommandSuccess=false;
    }
  else
    {
      ftpCommandSuccess=true;
    }

  p=qftpPtr->currentDevice();
  if(p)
    {
      delete qftpPtr->currentDevice();
    }

}


// This signal is emitted when the last pending command has finished;
// (it is emitted after the last command's commandFinished() signal).
// error is true if an error occurred during the processing; otherwise error is false.

void ftpThread::slotDone( bool error )
{
  //  timeoutTimer.stop();
  lastError=FTPERROR;
  if ( error )
    {
      // If we are connected, but not logged in, it is not meaningful to stay
      // connected to the server since the error is a really fatal one (login
      // failed).
      if(!isLoggedIn())
        {
          switch(qftpPtr->error())
            {
            case QFtp::NoError:
              lastError=FTPOK;
              break;
            case QFtp::UnknownError:
              break;
            case QFtp::HostNotFound:
              break;
            case QFtp::ConnectionRefused:
              break;
            case QFtp::NotConnected:
              break;
            }
          if(lastError==FTPOK)
            {
              emit commandsDone((int)lastError,"Success");
            }
          else
            {
              emit commandsDone((int)lastError,qftpPtr->errorString());
            }

          ftpCommandSuccess=false;
          addToLog(QString("'%1': error and not logged in-> disconnecting").arg(host),LOGFTPTHREAD);
          destroy();
          ftpDone=true;
          emit ftpQuit();
          return;
        }
      addToLog(QString("'%1': %2 error: %3").arg(idName).arg(host).arg(qftpPtr->errorString()),LOGFTPTHREAD);
    }
  else
    {
      addToLog(QString("'%1': %2 OK").arg(idName).arg(host),LOGFTPTHREAD);
    }
  if (!isUnconnected() && canCloseWhenDone)
    {
      addToLog(QString("'%1': schedule disconnect in %2ms").arg(idName).arg(disconnectTimerPtr->interval()),LOGFTPTHREAD);
      disconnectTimerPtr->start();
    }
  ftpCommandSuccess=true;
  ftpDone=true;
  lastError=FTPOK;
  emit commandsDone((int)lastError,"Success");
}



void ftpThread::slotListInfo( const QUrlInfo & ent)
{
  if (ent.isFile())
    {
      addToLog(QString("currentId=%1, name=%2").arg(qftpPtr->currentId()).arg(ent.name()), LOGFTPTHREAD);
      listingResults.append(ent);
    }
 }

void ftpThread::slotRawCommandReply( int code, const QString &text )
{
  Q_UNUSED(code);
  Q_UNUSED(text);
  addToLog(QString("FTP Raw Command Reply: code=%1 , %2").arg(code).arg(text),LOGFTPTHREAD);
}

void ftpThread::slotProgress(qint64 bytes ,qint64 total)
{
  if (displayProgress)
    {
      displayProgressFTPEvent *stmb;
      stmb=new displayProgressFTPEvent(bytes,total);
      QApplication::postEvent( dispatcherPtr, stmb );  // Qt will delete it when done
    }
  addToLog(QString("'%1' tim.restart interval=%2").arg(idName).arg(timeoutTimerPtr->interval()),LOGFTPTHREAD);
  timeoutTimerPtr->start();
}



