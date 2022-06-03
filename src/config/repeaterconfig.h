#ifndef REPEATERCONFIG_H
#define REPEATERCONFIG_H

#include "baseconfig.h"
#include "sstvparam.h"


extern bool repeaterEnabled;
extern int repeaterImageInterval;
extern esstvMode repeaterTxMode;
extern esstvMode repeaterIdleTxMode;
extern QString repeaterImage1;
extern QString repeaterImage2;
extern QString repeaterImage3;
extern QString repeaterImage4;
extern QString repeaterAcknowledge;
extern QString repeaterTemplate;
extern QString repeaterIdleTemplate;
extern int repeaterTxDelay;
extern int repeaterImageSize; // in kB

namespace Ui {
class repeaterConfig;
}

class repeaterConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit repeaterConfig(QWidget *parent = 0);
  ~repeaterConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

private slots:
  void slotRp1Browse();
  void slotRp2Browse();
  void slotRp3Browse();
  void slotRp4Browse();
  void slotRepeaterIdleTemplateBrowse();
  void slotRepeaterTemplateBrowse();
  
private:
  Ui::repeaterConfig *ui;
};

#endif // REPEATERCONFIG_H
