#ifndef WATERFALLCONFIG_H
#define WATERFALLCONFIG_H

#include "baseconfig.h"

extern QString startPicWF;
extern QString endPicWF;
extern QString fixWF;
extern QString bsrWF;
extern QString startBinWF;
extern QString endBinWF;
extern QString wfFont;
extern QString startRepeaterWF;
extern QString endRepeaterWF;
extern int wfFontSize;
extern bool wfBold;



namespace Ui {
class waterfallConfig;
}

class waterfallConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit waterfallConfig(QWidget *parent = 0);
  ~waterfallConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

public slots:
  void slotFontChanged();
  
private:
  Ui::waterfallConfig *ui;
};

#endif // WATERFALLCONFIG_H
