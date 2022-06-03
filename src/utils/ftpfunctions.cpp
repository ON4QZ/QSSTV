#include "ftpfunctions.h"
#include "appglobal.h"
#include "ftpevents.h"
#include "logging.h"
#include "configparams.h"
#include "dispatcher.h"

#include <QApplication>
#include <QDebug>
#include <QTemporaryFile>
#include <QMessageBox>
#include <errno.h>


ftpFunctions::ftpFunctions()
{
  thread=NULL;
  ftpThr=NULL;
  busy=false;
}

ftpFunctions::~ftpFunctions()
{

}

bool ftpFunctions::test(QString name,QString tHost,int tPort,QString tUser,QString tPasswd,QString tDirectory, bool doSetup)
{
  bool result=false;
  QString fn;
  QString rfn;
  QTemporaryFile tst;
  host=tHost;
  port=tPort;
  user=tUser;
  passwd=tPasswd;
  directory=tDirectory;
  idName=name;
  endOfCommands=false;
  addToLog(QString("%1 execFTPTest").arg(idName), LOGFTPFUNC);
  if (!tst.open())
    {
      lastError=FTPERROR;
      lastErrorStr=QString("Error writing temp file: %1, %2").arg(tst.fileName()).arg(strerror(errno));
    }
  else
    {
      if(doSetup) setupFtp();
      fn=tst.fileName();
      tst.write("connection test\n");
      tst.close();
      rfn = QString("test_%1.txt").arg(myCallsign);
      if((result=checkUpload(fn,rfn)))
        {
          result=checkRemove(rfn);
        }
    }

  if(result)
    {
      lastErrorStr=QString("Connection to %1/%2 OK").arg(host).arg(directory);
    }
//  if(lastTest)
//    {
//     thread->quit();
//    while(thread->isRunning())
//      {
//          QApplication::processEvents();
//       }
//    }
  return result;
}

bool ftpFunctions::checkUpload(QString fn, QString rfn)
{
  endOfCommands=false;
  ftp_uploadEvent *ftpUpl=new ftp_uploadEvent(fn,rfn,false);
  QApplication::postEvent(ftpThr,ftpUpl);
  checkWait(true);
  return lastError==FTPOK;
}


bool ftpFunctions::checkRemove(QString rfn)
{
  endOfCommands=false;
  ftp_removeEvent *ftpRemove=new ftp_removeEvent(rfn,true);
  QApplication::postEvent(ftpThr,ftpRemove);
  checkWait(true);
  return lastError==FTPOK;
}

bool ftpFunctions::checkStart()
{
  endOfCommands=false;
  if(ftpThr!=NULL)
    {
      return true;
    }
  return false;
}

void ftpFunctions::uploadFile(QString source,QString destination,bool wait,bool closeWhenDone)
{
  if (!checkStart()) return;
  ftp_uploadEvent *ftpUpl=new ftp_uploadEvent(source,destination,closeWhenDone);
  QApplication::postEvent(ftpThr,ftpUpl);
  checkWait(wait);
}


void ftpFunctions::downloadFile(QString source, QString destination,bool wait,bool closeWhenDone)
{
  if (!checkStart()) return;
  ftp_downloadEvent *ftpDwnl=new ftp_downloadEvent(source,destination,closeWhenDone);
  QApplication::postEvent(ftpThr,ftpDwnl);
  checkWait(wait);
}


void ftpFunctions::uploadData(QByteArray ba,QString destination,bool wait,bool closeWhenDone)
{
  QString fileName;
  if(!ftmp.open()) return ;
  ftmp.write(ba);
  ftmp.close();
  fileName=ftmp.fileName();
  uploadFile(fileName,destination,wait,closeWhenDone);

}

void ftpFunctions::remove(QString source,bool wait,bool closeWhenDone)
{
  if (!checkStart()) return;
  ftp_removeEvent *ftpRemove=new ftp_removeEvent(source,closeWhenDone);
  QApplication::postEvent(ftpThr,ftpRemove);
  checkWait(wait);
}

void ftpFunctions::mremove(QString mask,bool wait,bool closeWhenDone)
{
  if (!checkStart()) return;
  mremoveCmd=true;
  addToLog("mremove cmd",LOGFTPFUNC);
  ftp_listEvent *ftpList=new ftp_listEvent(mask,closeWhenDone);
  QApplication::postEvent(ftpThr,ftpList);
  checkWait(wait);
}


void ftpFunctions::rename(QString source, QString destination,bool wait,bool closeWhenDone)
{
  if (!checkStart()) return;
  ftp_renameEvent *ftpRename=new ftp_renameEvent(source,destination,closeWhenDone);
  QApplication::postEvent(ftpThr,ftpRename);
  checkWait(wait);
}

void ftpFunctions::changePath(QString source,bool wait)
{
  if (!checkStart()) return;
  addToLog(QString("changePath %1").arg(source),LOGFTPFUNC);
  ftp_cdEvent *ftpCd=new ftp_cdEvent(source);
  QApplication::postEvent(ftpThr,ftpCd);
  checkWait(wait);
}


void ftpFunctions::listFiles(QString mask,bool closeWhenDone)
{
  if (!checkStart()) return;
  ftp_listEvent *ftpList=new ftp_listEvent(mask,closeWhenDone);
  QApplication::postEvent(ftpThr,ftpList);
}

void ftpFunctions::setupFtp(QString name,QString tHost,int tPort,QString tUser,QString tPasswd,QString tDirectory)
{
  // create an ftp thread;
  host=tHost;
  port=tPort;
  user=tUser;
  passwd=tPasswd;
  directory=tDirectory;
  idName=name;
  setupFtp();
}

void ftpFunctions::changeThreadName(QString tidName)
{
  idName=tidName;
}

void ftpFunctions::setupFtp()
{
  thread = new QThread;
//  qDebug() << "creating thread" << idName;
  thread->setObjectName(idName);
  ftpThr=new ftpThread(idName);
  ftpThr->setHostParams(host,port,user,passwd,directory);
  ftpThr->moveToThread(thread);
  connect(thread, SIGNAL(started()), ftpThr, SLOT(slotInit()));
  connect(ftpThr, SIGNAL(commandsDone(int ,QString)), this,   SLOT(slotCommandsDone(int ,QString)));
  connect(ftpThr, SIGNAL(downloadFinished(bool,QString)), this,   SLOT(slotDownloadFinished(bool,QString)));
  connect(ftpThr, SIGNAL(listingComplete(bool)), this,   SLOT(slotListingFinished(bool)));
  //automatically delete thread and task object when work is done:
  connect( ftpThr, SIGNAL(ftpQuit()), thread, SLOT (quit()), Qt::DirectConnection);
  connect( thread, SIGNAL(finished()),  SLOT(slotThreadFinished()));
  thread->start();
  while(!thread->isRunning())
    {
      QApplication::processEvents();
    }
  busy=true;
  mremoveCmd=false;
}


void ftpFunctions::disconnectFtp()
{
  if(ftpThr)
    {
      ftp_disconnectEvent *ftpDisconnect=new ftp_disconnectEvent();
      QApplication::postEvent(ftpThr,ftpDisconnect);
      busy=false;
    }
}


void ftpFunctions::slotThreadFinished()
{
//  qDebug() << "slotThreadFished" << thread->objectName();
  thread->deleteLater();
//  ftpThr=NULL;
  busy=false;
}

void ftpFunctions::slotDownloadFinished(bool err,QString filename)
{
  emit downloadDone(err,filename);
}

void ftpFunctions::slotListingFinished(bool err)
{
  int i;
  QList <QUrlInfo> users;
  if(mremoveCmd)
    {
      users=getListing();
      for(i=0;i<users.count();i++)
        {
          remove(users.at(i).name(),false,false);
        }
      //     disconnectFtp();
    }
  emit listingDone(err);
}





QList<QUrlInfo> ftpFunctions::getListing()
{
  return ftpThr->getList();
}


void ftpFunctions::slotCommandsDone(int err,QString errStr)
{
  displayMBoxEvent *stmb;
  lastError=(eftpError) err;
  lastErrorStr=errStr;
  endOfCommands=true;
  addToLog("slotDone",LOGFTPFUNC);
  if(err)
    {
      stmb= new displayMBoxEvent("FTP Error", QString("Host %1\n%2").arg(host).arg(errStr));
      QApplication::postEvent( dispatcherPtr, stmb );
    }
}

void ftpFunctions::checkWait(bool wait)
{
  if(wait)
    {
      addToLog("start ftp wait",LOGFTPFUNC);
      while(!endOfCommands && busy)
        {
          qApp->processEvents();
        }
      addToLog(QString("ftp wait done %1 %2").arg(endOfCommands).arg(busy),LOGFTPFUNC);
    }
}
