#ifndef CWCONFIG_H
#define CWCONFIG_H

#include "baseconfig.h"

extern QString cwText;
extern int cwTone;
extern int cwWPM;
//extern bool enableCW;


namespace Ui {
class cwConfig;
}

class cwConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit cwConfig(QWidget *parent = 0);
  ~cwConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();
  
private:
  Ui::cwConfig *ui;
};

#endif // CWCONFIG_H
