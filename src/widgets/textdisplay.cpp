#include "textdisplay.h"
#include "ui_textdisplay.h"

textDisplay::textDisplay(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::textDisplay)
{
  ui->setupUi(this);
}

textDisplay::~textDisplay()
{
  delete ui;
}


void textDisplay::clear()
{
  ui->plainTextEdit->clear();
}

void textDisplay::append(QString t)
{
  int i;
  QStringList sl;
  sl=t.split("\r\n");
  for(i=0;i<sl.count();i++)
    {
      ui->plainTextEdit->appendPlainText(sl.at(i));
    }
}
