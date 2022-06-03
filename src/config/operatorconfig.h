#ifndef OPERATORCONFIG_H
#define OPERATORCONFIG_H
#include "baseconfig.h"

extern QString myCallsign;
extern QString myQth;
extern QString myLocator;
extern QString myLastname;
extern QString myFirstname;
extern QString lastReceivedCall;

extern bool    onlineStatusEnabled;
extern QString onlineStatusText;


namespace Ui {
class operatorConfig;
}

class operatorConfig : public baseConfig
{
  Q_OBJECT

public:
  explicit operatorConfig(QWidget *parent = 0);
  ~operatorConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

private:
  Ui::operatorConfig *ui;
};

#endif // OPERATORCONFIG_H
