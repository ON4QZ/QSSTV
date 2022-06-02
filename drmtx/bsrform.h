#ifndef BSRFORM_H
#define BSRFORM_H

#include <QDialog>
#include "sourcedecoder.h"
#include "drmtransmitter.h"

namespace Ui {
class bsrForm;
}

class bsrForm : public QDialog
{
  Q_OBJECT

  
public:
  enum eResult {CANCEL,EASYPAL,COMPAT};
  explicit bsrForm(QWidget *parent = 0);
  ~bsrForm();
  void init();
  QByteArray *getBA(bool compat);
  bool hasBSR();
  drmTxParams getDRMParams() {return  drmParams;}
public slots:
  void slotCompatible();
  void slotEasypal();
  void slotCanceled();
  
private slots:
 void slotBSRSelection(int);

private:
  Ui::bsrForm *ui;
  void displayInfo(int idx);
  QList <bsrBlock> *bsrPtr;
  drmTxParams drmParams;

};

#endif // BSRFORM_H
