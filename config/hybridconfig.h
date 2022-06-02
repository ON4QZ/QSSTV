#ifndef HYBRIDCONFIG_H
#define HYBRIDCONFIG_H

#include "baseconfig.h"

extern bool enableHybridRx;

extern int hybridFtpPort;
extern QString hybridFtpRemoteHost;
extern QString hybridFtpRemoteDirectory;
extern QString hybridFtpLogin;
extern QString hybridFtpPassword;
extern QString hybridFtpHybridFilesDirectory;

extern bool enableHybridNotify;
extern QString hybridNotifyDir;
extern QString onlineStatusDir;


namespace Ui {
class hybridConfig;
}

class hybridConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit hybridConfig(QWidget *parent = 0);
  ~hybridConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();
private slots:
  void slotTestHybridPushButton();
  
private:
  Ui::hybridConfig *ui;
};

#endif // HYBRIDCONFIG_H
