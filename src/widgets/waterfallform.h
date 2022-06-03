#ifndef WATERFALLFORM_H
#define WATERFALLFORM_H

#include <QDialog>

namespace Ui {
  class waterfallForm;
  }

class waterfallForm : public QDialog
{
  Q_OBJECT
  
public:
  explicit waterfallForm(QWidget *parent = 0);
  ~waterfallForm();
  QString text(){ return txt;}

private slots:
  void slotText1();
  void slotText2();
  void slotText3();
  void slotText4();
private:
  Ui::waterfallForm *ui;
  QString txt1;
  QString txt2;
  QString txt3;
  QString txt4;
  QString txt;
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();
  void accept();
};

#endif // WATERFALLFORM_H
