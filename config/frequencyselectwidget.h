#ifndef FREQUENCYSELECTWIDGET_H
#define FREQUENCYSELECTWIDGET_H
#include "baseconfig.h"

#include <QWidget>

extern QStringList freqList;
extern QStringList modeList;
extern QStringList sbModeList;
extern QStringList passBandList;
extern QString additionalCommand;
extern bool additionalCommandHex;

namespace Ui {
class frequencySelectWidget;
}

class frequencySelectWidget : public baseConfig
{
  Q_OBJECT

public:
  explicit frequencySelectWidget(QWidget *parent = 0);
  ~frequencySelectWidget();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

private slots:
  void slotFreqAdd();
  void slotFreqDelete();
  void slotFreqUp();
  void slotFreqDown();
  void slotItemChanged();
  void slotCellClicked(int r, int);

private:
  Ui::frequencySelectWidget *ui;
  void constructTable();
  void createEntry(int row);
  void setLastRowSelected();
  int lastRowSelected;

};

#endif // FREQUENCYSELECTWIDGET_H
