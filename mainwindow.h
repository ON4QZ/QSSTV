#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

class configDialog;
class spectrumWidget;
class ftpThread;

namespace Ui {
  class MainWindow;
  }

class mainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit mainWindow(QWidget *parent = 0);
  ~mainWindow();
  void init();
  void startRunning();
  void setNewFont();
  void setPTT(bool p);
  void setSSTVDRMPushButton(bool inDRM);
  spectrumWidget *spectrumFramePtr;

private slots:
  void slotConfigure();
  void slotSaveWaterfallImage();
  void slotExit();
  void slotResetLog();
  void slotLogSettings();
  void slotAboutQt();
  void slotAboutQSSTV();
  void slotFullScreen();
  void slotDocumentation();
  void slotCalibrate();
  void slotModeChange(int);
  void slotSendWFID();
  void slotSendCWID();
  void slotSendBSR();
  void slotSendWfText();
  void slotSetFrequency(int freqIndex);



#ifndef QT_NO_DEBUG
  void slotShowDataScope();
  void slotShowSyncScopeNarrow();
  void slotShowSyncScopeWide();
  void slotScopeOffset();
  void slotClearScope();
  void slotDumpSamplesPerLine();
  void slotTxTestPattern();
#endif

private:
  Ui::MainWindow *ui;
  void closeEvent ( QCloseEvent *e );
  void readSettings();
  void writeSettings();
  void restartSound(bool inStartUp);
  void cleanUpCache(QString dirPath);
//  void setupFtp(ftpThread *&ptr, QString idName);
  QComboBox *transmissionModeComboBox;
  QPushButton *wfTextPushButton;
  QPushButton *fixPushButton;
  QPushButton *bsrPushButton;
  QPushButton *idPushButton;
  QPushButton *cwPushButton;
  QComboBox *freqComboBox;
  QLabel pttText;
  QLabel *pttIcon;
  QLabel *freqDisplay;
  void timerEvent(QTimerEvent *);
  QStringList modModeList;
  QStringList modPassBandList;
};

#endif // MAINWINDOW_H
