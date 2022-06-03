#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class configDialog;
}

class configDialog : public QDialog
{
  Q_OBJECT

public:
  explicit configDialog(QWidget *parent = 0);
  ~configDialog();
  void readSettings();
  void writeSettings();
  int exec();
  bool soundNeedsRestart;
  bool guiNeedsRestart;


signals:
  void bgColorChanged();


private:
  Ui::configDialog *ui;

};

#endif // CONFIGDIALOG_H
