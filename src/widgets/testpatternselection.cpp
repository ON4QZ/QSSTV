#include "testpatternselection.h"
#include "ui_testpatternselection.h"

testPatternSelection::testPatternSelection(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::testPatternSelection)
{
  ui->setupUi(this);
}

testPatternSelection::~testPatternSelection()
{
  delete ui;
}

etpSelect testPatternSelection::getSelection()
{
 if(ui->rasterRadioButton->isChecked()) return TPRASTER;
 else if(ui->whiteRadioButton->isChecked()) return TPWHITE;
 else if(ui->blackRadioButton->isChecked()) return TPBLACK;
 return TPGRAY;
}
