#ifndef DISPATCHEVENT_H
#define DISPATCHEVENT_H
#include <QEvent>
#include "imageviewer.h"
#include "appdefs.h"
#include "hybridcrypt.h"
#include <unistd.h>
#include "sstvparam.h"

class ftpThread;


/** dispatch events are used to communicate with the different threads */
enum dispatchEventType
{
  info = QEvent::User, //!< send when dsp stops running
  soundcardIdle, //!< send when soundcard stops running
  displayFFT,
  displaySync,
  displayDRMStat,
  displayDRMInfo,
  syncDisp,				//!< synchro display event
  lineDisplay,				//!< display 1 line
  eraseDisp,
  createMode,
  startImageRX,
  endSSTVImageRX,
  endImageTX,
  stoppingTX,
  progressTX,
  outOfSync,
  rxSSTVStatus,     //! shows message in sstv tab
  rxDRMStatus,     //! shows message in drm tab
  txDRMNotify,  //! shows text in tx notifications box
  txDRMNotifyAppend,
  txPrepareComplete, //!< tx preparations (uploading etc) complete
  closeWindows,
  callEditor,
  templatesChanged,
  editorFinished,
  changeRXFilter,
//  startAutoRepeater,
//  startRepeater,
  stopRxTx,
  loadRXImage,
  saveDRMImage,
  prepareFix,
  displayText,
  displayMBox,
  displayProgressFTP,
  moveToTx,
//  notifyAction,
  ftpSetup,
  notifyCheck,
  ftpUploadFile,
  statusBarMsg
};

class baseEvent: public QEvent
{
public:
  baseEvent(QEvent::Type t):QEvent(t) {doneIt=NULL;}
  void waitFor(bool *d) {doneIt=d;}
  void setDone()
  {
    if(doneIt!=NULL) *doneIt=true;
  }
  QString description;
private:
  bool *doneIt;

};

/**
  this event is send when the dspfunc thread stops running
*/
class infoEvent : public  baseEvent
{
public:
  /** create event */
  infoEvent(QString t):baseEvent( (QEvent::Type) info ), str(t)
  {
    description="infoEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};

/**
  this event is send when the soundcard thread goes to idle
*/
class soundcardIdleEvent : public baseEvent
{
public:
  /** create event */
  soundcardIdleEvent():baseEvent( (QEvent::Type) soundcardIdle )
  {
    {
      description=" soudcardIdleEvent";
    }
  }
};


/**
  this event is send with teh sync quality info and the signal volume
*/
class displaySyncEvent : public baseEvent
{
public:
  /** create event */
  displaySyncEvent(uint s):baseEvent( (QEvent::Type) displaySync), sync(s)
  {
    description=" displaySyncEvent";
  }
  /** returns int sync value */
  void getInfo(uint &s)  {s=sync;}

private:
  uint sync;
//  DSPFLOAT vol;
};

class displayDRMStatEvent  : public baseEvent
{
public:
  /** create event */
  displayDRMStatEvent(uint s):baseEvent( (QEvent::Type) displayDRMStat), snr(s)
  {
    description=" displayDRMStatEvent";
  }
  /** returns length and pointer  from the event */
  void getInfo(DSPFLOAT &s)  {s=snr;}

private:
  DSPFLOAT snr;
};


class ftpSetupEvent : public baseEvent
{
public:
  /** create event */
  ftpSetupEvent(ftpThread * ftpIntf,QString h,int p,QString u,QString pwd,QString d)
    :baseEvent( (QEvent::Type)ftpSetup ),ftpIntfPtr(ftpIntf), host(h),port(p),user(u),password(pwd),dir(d)
  {
    description="ftpSetupEvent";
  }
  /** returns settings from the event */
  ftpThread *getFtpIntfPtr() const { return ftpIntfPtr; }
  QString getHost() const { return host; }
  int getPort() const { return port; }
  QString getUser() const { return user; }
  QString getPassword() const { return password; }
  QString getDir() const { return dir; }
private:
  ftpThread *ftpIntfPtr;
  QString host;
  int port;
  QString user;
  QString password;
  QString dir;
};

class ftpUploadFileEvent : public baseEvent
{
public:
  /** create event */
  ftpUploadFileEvent(ftpThread * ftpIntf,QString srcFn,QString dstFn,bool recon)
    :baseEvent( (QEvent::Type)ftpUploadFile ),ftpIntfPtr(ftpIntf),sourceFilename(srcFn), destFilename(dstFn),reconnect(recon)
  {
    description="ftpUploadFileEvent";
  }
  /** returns settings from the event */
  ftpThread *getFtpIntfPtr() const { return ftpIntfPtr; }
  QString getSrcFn() const { return sourceFilename; }
  QString getDstFn() const { return destFilename; }
  bool   getReconnect() const { return reconnect; }
private:
  ftpThread *ftpIntfPtr;
  QString sourceFilename;
  QString destFilename;
  bool reconnect;
};



//class notifyActionEvent : public baseEvent
//{
//public:
//  /** create event */
//  notifyActionEvent(hybridCrypt thc,QString toRem,QString message,QString fn)
//    :baseEvent( (QEvent::Type)notifyAction ),hc(thc), toRemove(toRem),msg(message),filename(fn)
//  {
//    description="notifyActionEvent";
//  }
//  /** returns settings from the event */
//  hybridCrypt getHybridCrypt() const { return hc; }
//  QString getToRemove() const { return toRemove; }
//  QString getMsg() const { return msg; }
//  QString getFilename() const { return filename; }
//private:
//  hybridCrypt hc;
//  QString toRemove;
//  QString msg;
//  QString filename;
//};


class notifyCheckEvent : public baseEvent
{
public:
  /** create event */
  notifyCheckEvent(QString fn):baseEvent( (QEvent::Type)notifyCheck),filename(fn)
  {
    description="notifyCheckEvent";
  }
  QString getFilename() const { return filename; }
private:
  QString filename;
};



class rxSSTVStatusEvent : public baseEvent
{
public:
  /** create event */
  rxSSTVStatusEvent(QString t):baseEvent( (QEvent::Type)rxSSTVStatus ), str(t)
  {
    description="rxSSTVStatusEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};

class rxDRMStatusEvent : public baseEvent
{
public:
  /** create event */
  rxDRMStatusEvent(QString t):baseEvent( (QEvent::Type)rxDRMStatus ), str(t)
  {
    description="rxDRMStatusEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};



class txDRMNotifyEvent : public baseEvent
{
public:
  /** create event */
  txDRMNotifyEvent(QString t):baseEvent( (QEvent::Type)txDRMNotify ), str(t)
  {
    description="txDRMNotifyEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};

class txDRMNotifyAppendEvent : public baseEvent
{
public:
  /** create event */
  txDRMNotifyAppendEvent(QString t):baseEvent( (QEvent::Type)txDRMNotifyAppend ), str(t)
  {
    description="txDRMNotifyAppendEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};

class txPrepareCompleteEvent : public baseEvent
{
public:
  /** create event */
  txPrepareCompleteEvent(bool tok):baseEvent( (QEvent::Type)txPrepareComplete ), _ok(tok)
  {
    description="txPrepareCompleteEvent";
  }
  /** returns info string from the event */
  bool ok() const { return _ok; }
private:
  bool _ok;
};

class lineDisplayEvent : public baseEvent
{
public:
  /** create event */
  lineDisplayEvent(uint lineNbr):baseEvent( (QEvent::Type)lineDisplay ), lineNumber(lineNbr)
  {
    description="lineDisplayEvent";
  }
  /** returns length and pointer  from the event */
  void getInfo(uint &lineNbr) const { lineNbr=lineNumber;}

private:
  uint lineNumber;
};

class eraseDisplayEvent : public baseEvent
{
public:
  /** create event */
  eraseDisplayEvent():baseEvent( (QEvent::Type)eraseDisp )
  {
    description="eraseDisplayEvent";
  }
};



class displayDRMInfoEvent : public baseEvent
{
public:
  /** create event */
  displayDRMInfoEvent():baseEvent( (QEvent::Type)displayDRMInfo)
  {
    description="displayDRMInfo";
  }
};

//class startAutoRepeaterEvent: public baseEvent
//{
//public:
//  /** create event */
//  startAutoRepeaterEvent():baseEvent( (QEvent::Type)startAutoRepeater )
//  {
//    description="startAutoRepeaterEvent";
//  }
//};

//class startRepeaterEvent: public baseEvent
//{
//public:
//  /** create event */
//  startRepeaterEvent():baseEvent( (QEvent::Type)startRepeater )
//  {
//    description="startRepeaterEvent";
//  }
//};


class createModeEvent : public baseEvent
{
public:
  /** create event */
  createModeEvent(uint m,QString t):baseEvent( (QEvent::Type)createMode ), mode(m) ,str(t)
  {
    description="createModeEvent";
  }
  /** returns info string from the event */
  void getMode(uint &m,QString &s) const { m=mode;s=str; }
private:
  uint mode;
  QString str;
};

class loadRXImageEvent : public baseEvent
{
public:
  loadRXImageEvent(QString fn):baseEvent( (QEvent::Type)loadRXImage),fileName(fn)
  {
    description="loadRXImageEvent";
  }
  QString getFilename() {return fileName;}
private:
  QString fileName;
};


class moveToTxEvent : public baseEvent
{
public:
  moveToTxEvent(QString fn):baseEvent( (QEvent::Type)moveToTx),fileName(fn)
  {
    description="moveToTxEvent";
  }
  QString getFilename() {return fileName;}
private:
  QString fileName;
};



class saveDRMImageEvent : public baseEvent
{
public:
  saveDRMImageEvent(QString fn,QString i):baseEvent( (QEvent::Type)saveDRMImage),fileName(fn),info(i)
  {
    description="saveDRMImageEvent";
  }
  void getFilename(QString &fn) {fn=fileName;}
  void getInfo(QString &i) {i=info;}
private:
  QString fileName;
  QString info;
};



class startImageRXEvent : public baseEvent
{
public:
  /** create event */
  startImageRXEvent(QSize ims):baseEvent( (QEvent::Type)startImageRX ),imSize(ims)
  {
    description="startImageRXEvent";
  }
  QSize getSize()  {return imSize;}
private:
  QSize imSize;

};

class endImageSSTVRXEvent : public baseEvent
{
public:
  /** create event */
  endImageSSTVRXEvent(esstvMode md):baseEvent( (QEvent::Type)endSSTVImageRX ),mode(md)
  {
    description="endImageSSTVRXEvent";
  }
  esstvMode getMode() {return mode;}
private:
  esstvMode mode;
};

class endImageTXEvent : public baseEvent
{
public:
  /** create event */
  endImageTXEvent():baseEvent( (QEvent::Type)endImageTX )
  {
    description="endImageTXEvent";
  }
};


class stopTXEvent : public baseEvent
{
public:
  /** create event */
  stopTXEvent():baseEvent( (QEvent::Type)stoppingTX )
  {
    description="stopTXEvent";
  }
};




class outOfSyncEvent : public baseEvent
{
public:
  /** create event */
  outOfSyncEvent():baseEvent( (QEvent::Type)outOfSync )
  {
    description="outOfSyncEvent";
  }
};



class progressTXEvent : public baseEvent
{
public:
  /** create event */
  progressTXEvent(double tim):baseEvent( (QEvent::Type)progressTX ), txTime(tim)
  {
    description="progressTXEvent";
  }
  /** returns length and pointer  from the event */
  double getInfo() { return txTime;}

private:
  double txTime;
};

class closeWindowsEvent : public baseEvent
{
public:
  /** create event */
  closeWindowsEvent():baseEvent( (QEvent::Type)closeWindows)
  {
    description="closeWindowEvent";
  }
  /** returns length and pointer  from the event */
};



class callEditorEvent : public baseEvent
{
public:
  /** create event */
  callEditorEvent(imageViewer *iv,QString fn):baseEvent( (QEvent::Type) callEditor ), filename(fn),imviewer(iv)
  {
    description="callEditorEvent";
  }
  /** returns info string from the event */
  QString getFilename() const { return filename; }
  imageViewer *getImageViewer() { return imviewer; }
private:
  QString filename;
  imageViewer *imviewer;
};


class templatesChangedEvent : public baseEvent
{
public:
  /** create event */
  templatesChangedEvent():baseEvent( (QEvent::Type) templatesChanged )
  {
    description="templateChangeEvent";
  }
};

class editorFinishedEvent : public baseEvent
{
public:
  /** create event */
  editorFinishedEvent(bool b,QString fn):baseEvent( (QEvent::Type)editorFinished),ok(b),filename(fn)
  {
    description="editorFinishedEvent";
  }
  bool isOK() { return ok;}
  QString getFilename() const { return filename; }

private:
  bool ok;
  QString filename;

};


class displayFFTEvent : public baseEvent
{
public:
  /** create event */
  displayFFTEvent(DSPFLOAT *buf):baseEvent( (QEvent::Type)displayFFT),buffer(buf)
  {
    description="displayFFTEvent";
  }
  DSPFLOAT *data() { return buffer;}
private:
  DSPFLOAT *buffer;

};

class filterRXChangedEvent: public baseEvent
{
public:
  /** create event */
  filterRXChangedEvent(int fIndex):baseEvent( (QEvent::Type)changeRXFilter),filterIndex(fIndex)
  {
    description="filterChangedEvent";
  }
  int index() { return filterIndex;}
private:
  int filterIndex;
};

class stopRxTxEvent : public baseEvent
{
public:
  /** create event */
  stopRxTxEvent():baseEvent( (QEvent::Type)stopRxTx)
  {
    description="stopRxTxEvent";
  }
};


class prepareFixEvent: public baseEvent
{
public:
  prepareFixEvent(QByteArray ba):baseEvent( (QEvent::Type)prepareFix),data(ba)
  {
    description="filterChangedEvent";
  }
  QByteArray &getData() {return data;}
private:
  QByteArray data;
};


/**
  this event is send when the dspfunc thread stops running
*/
class displayTextEvent : public  baseEvent
{
public:
  /** create event */
  displayTextEvent(QString t):baseEvent( (QEvent::Type) displayText ), str(t)
  {
    description="displayTextEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};

class displayMBoxEvent : public  baseEvent
{
public:
  /** create event */
  displayMBoxEvent(QString title,QString text):baseEvent( (QEvent::Type) displayMBox ), str(text), title(title)
  {
    description="displayMBoxEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
  QString getTitle() const { return title; }

private:
  QString str;
  QString title;
};


class displayProgressFTPEvent : public  baseEvent
{
public:
  /** create event */
  displayProgressFTPEvent(quint64 byts,quint64 tot):baseEvent( (QEvent::Type) displayProgressFTP ),  bytes(byts),total(tot)
  {
    description="displayMBoxEvent";
  }
  /** returns info string from the event */
  quint64 getTotal() const { return total; }
  quint64 getBytes() const { return bytes; }

private:
  quint64 bytes;
  quint64 total;

};

class statusBarMsgEvent : public baseEvent
{
public:
  /** create event */
  statusBarMsgEvent(QString t):baseEvent( (QEvent::Type)statusBarMsg ), str(t)
  {
    description="statusBarMsgEvent";
  }
  /** returns info string from the event */
  QString getStr() const { return str; }
private:
  QString str;
};

#endif
