#ifndef APPGLOBAL_H
#define APPGLOBAL_H
#include "logging.h"
//#include "mainwindow.h"
#include "appdefs.h"

class QSplashScreen;
class scopeView;
class rxWidget;
class txWidget;
class galleryWidget;
class waterfallText;
class rigControl;
class xmlInterface;
class logBook;
class dispatcher;
class fileWatcher;
class ftpThread;

enum etransmissionMode {TRXSSTV,TRXDRM,TRXNOMODE};

extern const QString MAJORVERSION;
extern const QString CONFIGVERSION;
extern const QString MINORVERSION;
extern  const QString ORGANIZATION;
extern const QString APPLICATION;
extern const QString qsstvVersion;
extern const QString APPNAME;
#define MAGICNUMBER   (('4'<<24)+('Q'<<16)+('Z'<<8)+'S')

class soundBase;
class mainWindow;
class QStatusBar;
class configDialog;

extern mainWindow *mainWindowPtr;
extern soundBase *soundIOPtr;
extern QSplashScreen *splashPtr;
extern QString splashStr;
extern dispatcher *dispatcherPtr;
extern QStatusBar *statusBarPtr;
extern fileWatcher *fileWatcherPtr;

extern rxWidget *rxWidgetPtr;
extern txWidget *txWidgetPtr;
extern galleryWidget *galleryWidgetPtr;
extern waterfallText *waterfallPtr;
extern rigControl *rigControllerPtr;
extern xmlInterface *xmlIntfPtr;
extern configDialog *configDialogPtr;
extern logBook *logBookPtr;

extern int fftNumBlocks;


extern QPixmap *greenPXMPtr;
extern QPixmap *redPXMPtr;

extern logFile *logFilePtr;
extern bool useHybrid;
extern bool inStartup;


extern ftpThread *notifyRXIntfPtr;
extern ftpThread *hybridTxIntfPtr;
extern ftpThread *notifyTXIntfPtr;
extern ftpThread *onlineStatusIntfPtr;
extern ftpThread *hybridRxIntfPtr;
extern ftpThread *saveImageIntfPtr;

extern etransmissionMode transmissionModeIndex;  // SSTV , DRM


#ifndef QT_NO_DEBUG
extern scopeView *scopeViewerData;
extern scopeView *scopeViewerSyncNarrow;
extern scopeView *scopeViewerSyncWide;
#endif

void globalInit(void);
void globalEnd(void);

#endif // APPGLOBAL_H
