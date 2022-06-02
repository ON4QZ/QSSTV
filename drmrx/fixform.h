#ifndef FIXFORM_H
#define FIXFORM_H

#include <QDialog>

namespace Ui {
class fixForm;
}

class fixForm : public QDialog
{
  Q_OBJECT
  
public:
  explicit fixForm(QWidget *parent = 0);
  ~fixForm();
  void setInfoInternal(int mode, QString fileName, int missing, QByteArray *ba);
//  void setInfoExternal(int mode, QString fileName, int missing);
  
private:
  Ui::fixForm *ui;
  void common(int mode, QString fileName, int missing);
};

#endif // FIXFORM_H
