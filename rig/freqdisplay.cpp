#include "freqdisplay.h"
#include "ui_freqdisplay.h"

freqDisplay::freqDisplay(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::freqDisplay)
{
  ui->setupUi(this);
}

freqDisplay::~freqDisplay()
{
  delete ui;
}

void freqDisplay::display(double freq)
{
  ui->frequencyLCD->display(QString("%1").arg(freq/1000,8,'f',3));
}
