#ifndef FTPEVENTS_H
#define FTPEVENTS_H

#include <QString>
#include <QEvent>


enum ftpEventType
{
  ftp_list=QEvent::User,
  ftp_remove,
  ftp_rename,
  ftp_upload,
  ftp_download,
  ftp_disconnect,
  ftp_cd
};


class ftpBaseEvent: public QEvent
{
public:
  ftpBaseEvent(QEvent::Type t):QEvent(t) {doneIt=NULL;}
  void waitFor(bool *d) {doneIt=d;}
  void setDone()
  {
    if(doneIt!=NULL) *doneIt=true;
  }
  QString description;
  bool closeWhenDone;
  QString source;
  QString destination;
private:
  bool *doneIt;
};

class ftp_downloadEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_downloadEvent(QString tSource,QString tDestination,bool closeDone):ftpBaseEvent( (QEvent::Type) ftp_download)
  {
    description="ftp download";
    closeWhenDone=closeDone;
    source=tSource;
    destination=tDestination;
  }
};

class ftp_uploadEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_uploadEvent(QString tSource,QString tDestination,bool closeDone):ftpBaseEvent( (QEvent::Type) ftp_upload)
  {
    description="ftp upload";
    closeWhenDone=closeDone;
    source=tSource;
    destination=tDestination;
  }
};

class ftp_renameEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_renameEvent(QString tSource,QString tDestination,bool closeDone):ftpBaseEvent( (QEvent::Type) ftp_rename)
  {
    description="ftp rename";
    closeWhenDone=closeDone;
    source=tSource;
    destination=tDestination;
  }
};



class ftp_removeEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_removeEvent(QString tSource,bool closeDone):ftpBaseEvent( (QEvent::Type) ftp_remove )
  {
    description="ftp remove";
    closeWhenDone=closeDone;
    source=tSource;
    destination="";
  }
};

class ftp_listEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_listEvent(QString tMask,bool closeDone):ftpBaseEvent( (QEvent::Type) ftp_list )
  {
    description="ftp list";
    closeWhenDone=closeDone;
    source=tMask;
    destination="";
  }
};


class ftp_cdEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_cdEvent(QString dir):ftpBaseEvent( (QEvent::Type) ftp_cd )
  {
    description="ftp cd";
    closeWhenDone=false;
    source=dir;
    destination="";
  }
};


class ftp_disconnectEvent : public  ftpBaseEvent
{
public:
  /** create event */
  ftp_disconnectEvent():ftpBaseEvent( (QEvent::Type) ftp_disconnect)
  {
    description="ftp disconnect";
  }
};


#endif // FTPEVENTS_H
