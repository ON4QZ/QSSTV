#include "canvassizeform.h"
#include "ui_canvassizeform.h"
#include "supportfunctions.h"

canvasSizeForm::canvasSizeForm(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::canvasSizeForm)
{
  ui->setupUi(this);
}

canvasSizeForm::~canvasSizeForm()
{
  delete ui;
}

QRect canvasSizeForm::getSize()
{
  int x;
  int y;
  getValue(x,ui->widthSpinBox);
  getValue(y,ui->heightSpinBox);
  return QRect(0,0,x,y);
}

void canvasSizeForm::setSize(int x,int y)
{
  setValue(x,ui->widthSpinBox);
  setValue(y,ui->heightSpinBox);
}
