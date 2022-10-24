#include "scopeoffset.h"
#include "ui_scopeoffset.h"


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

void scopeOffset::setNumSamples(uint numSamples)
{
  ui->numberOfSamplesSpinbox->setValue(numSamples);
}

uint scopeOffset::getNumSamples()
{
  return ui->numberOfSamplesSpinbox->value();
}

void scopeOffset::setOffset(unsigned int offset)
{
  ui->offsetSpinbox->setValue(offset);
}

unsigned int scopeOffset::getOffset()
{
  return ui->offsetSpinbox ->value();
}


