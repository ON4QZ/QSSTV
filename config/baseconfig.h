#ifndef BASECONFIG_H
#define BASECONFIG_H

#include <QString>
#include <QWidget>
#include <QSettings>
#include "supportfunctions.h"

class baseConfig: public QWidget
{
public:
  baseConfig(QWidget *parent = 0);
  virtual void readSettings()=0;
  virtual void writeSettings()=0;
  virtual void getParams()=0;
  virtual void setParams()=0;
  bool hasChanged() {return changed;}
protected:
  bool changed;
};

#endif // BASECONFIG_H
