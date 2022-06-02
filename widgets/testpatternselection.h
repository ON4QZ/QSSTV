#ifndef TESTPATTERNSELECTION_H
#define TESTPATTERNSELECTION_H

#include <QDialog>

enum etpSelect {TPBLACK,TPWHITE,TPGRAY,TPRASTER};

namespace Ui {
class testPatternSelection;
}

class testPatternSelection : public QDialog
{
  Q_OBJECT

public:
  explicit testPatternSelection(QWidget *parent = 0);
  ~testPatternSelection();
  etpSelect getSelection();

private:
  Ui::testPatternSelection *ui;
};

#endif // TESTPATTERNSELECTION_H
