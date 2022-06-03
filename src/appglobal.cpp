#include "appglobal.h"
#include "logging.h"
#include "soundbase.h"
#include <QPixmap>
#include <QCursor>


const QString MAJORVERSION  = "9.5";
const QString CONFIGVERSION = "9.0";
const QString MINORVERSION  = ".11";
const QString LOGVERSION = ("qsstv."+MAJORVERSION+MINORVERSION+".log");
const QString ORGANIZATION = "ON4QZ";
const QString APPLICATION  = ("qsstv_" +CONFIGVERSION);
const QString qsstvVersion=QString("QSSTV " + MAJORVERSION+MINORVERSION);
const QString APPNAME=QString("QSSTV");


QSplashScreen *splashPtr;
QString splashStr;

mainWindow *mainWindowPtr;
soundBase *soundIOPtr;
logFile *logFilePtr;
configDialog *configDialogPtr;


ftpThread *notifyRXIntfPtr;
ftpThread *hybridTxIntfPtr;
ftpThread *notifyTXIntfPtr;
ftpThread *onlineStatusIntfPtr;
ftpThread *hybridRxIntfPtr;
ftpThread *saveImageIntfPtr;




dispatcher *dispatcherPtr;
QStatusBar *statusBarPtr;
rxWidget *rxWidgetPtr;
txWidget *txWidgetPtr;
galleryWidget *galleryWidgetPtr;
waterfallText *waterfallPtr;
rigControl *rigControllerPtr;
xmlInterface *xmlIntfPtr;
logBook *logBookPtr;
fileWatcher *fileWatcherPtr;


int fftNumBlocks=2;
bool useHybrid;
bool inStartup;






etransmissionMode transmissionModeIndex;  // SSTV , DRM



QPixmap *greenPXMPtr;
QPixmap *redPXMPtr;

#ifndef QT_NO_DEBUG
scopeView *scopeViewerData;
scopeView *scopeViewerSyncNarrow;
scopeView *scopeViewerSyncWide;
#endif


void globalInit()
{
  logFilePtr=new logFile();
  logFilePtr->open(LOGVERSION);
  QSettings qSettings;
  qSettings.beginGroup("MAIN");
  logFilePtr->readSettings();
  greenPXMPtr=new QPixmap(16,16);
  greenPXMPtr->fill(Qt::green);
  redPXMPtr=new QPixmap(16,16);
  redPXMPtr->fill(Qt::red);
  qSettings.endGroup();
}

void globalEnd(void)
{
  logFilePtr->writeSettings();
  logFilePtr->close();
}

