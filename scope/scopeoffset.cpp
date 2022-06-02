#include "scopeoffset.h"
#include "ui_scopeoffset.h"

uint dataScopeOffset;

scopeOffset::scopeOffset(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::scopeOffset)
{
  ui->setupUi(this);
}

scopeOffset::~scopeOffset()
{
  delete ui;
}


void scopeOffset::setOffset(unsigned int offset)
{
  ui->spinBox->setValue(offset);
}

unsigned int scopeOffset::getOffset()
{
  return ui->spinBox->value();
}


