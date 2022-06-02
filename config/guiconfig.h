#ifndef GUICONFIG_H
#define GUICONFIG_H

#include <QWidget>
#include "baseconfig.h"

extern int galleryRows;
extern int galleryColumns;
extern bool imageStretch;
extern QString defaultImageFormat;
extern QColor backGroundColor;
extern QColor imageBackGroundColor;
extern bool slowCPU;
extern bool lowRes;
extern bool confirmDeletion;
extern bool confirmClose;

namespace Ui
{
class guiConfig;
}

class guiConfig :public baseConfig
{
  Q_OBJECT
  
public:
  explicit guiConfig(QWidget *parent = 0);
  ~guiConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();
  bool backGroundColorChanged;
private slots:
  void slotBGColorSelect();
  void slotIBGColorSelect();

private:
  Ui::guiConfig *ui;
  void setColorLabel(QColor c, bool image);

};

#endif // GUICONFIG_H
