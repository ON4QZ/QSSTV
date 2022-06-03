#ifndef FTPCONFIG_H
#define FTPCONFIG_H


#include "baseconfig.h"


#define NUMFTPRXIMAGES 30

enum eftpSaveFormat {FTPIMAGESEQUENCE,FTPFILENAME};

extern bool enableFTP;
extern int ftpPort;
extern QString ftpRemoteHost;
extern QString ftpRemoteSSTVDirectory;
extern QString ftpRemoteDRMDirectory;
extern QString ftpLogin;
extern QString ftpPassword;
extern QString ftpFilename;
extern QString ftpDefaultImageFormat;
extern eftpSaveFormat ftpSaveFormat;
extern bool addExtension;

extern int ftpNumImages;


namespace Ui {
class ftpConfig;
}

class ftpConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit ftpConfig(QWidget *parent = 0);
  ~ftpConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

private slots:
  void slotTestFTPPushButton();
  
private:
  Ui::ftpConfig *ui;
};

#endif // FTPCONFIG_H
