#ifndef RIGCONFIG_H
#define RIGCONFIG_H

#include "baseconfig.h"

class rigControl;
struct scatParams;


namespace Ui {
class rigConfig;
}

class rigConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit rigConfig(QWidget *parent = 0);
  ~rigConfig();
  void attachRigController(rigControl *rigCtrl);
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

public slots:
  void slotEnableCAT();
  void slotEnablePTT();
  void slotEnableXMLRPC();
  void slotRestart();
  void slotCheckPTT0();
  void slotCheckPTT1();
  void slotCheckPTT2();
  void slotCheckPTT3();
  
private:

  Ui::rigConfig *ui;
  scatParams *cp;
  rigControl *rigController;
  void checkPTT(int p,bool b);
};

#endif // RIGCONFIG_H
