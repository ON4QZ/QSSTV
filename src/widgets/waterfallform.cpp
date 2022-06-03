#include "waterfallform.h"
#include "ui_waterfallform.h"
#include "appglobal.h"
#include "waterfalltext.h"


waterfallForm::waterfallForm(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::waterfallForm)
{
  ui->setupUi(this);
  connect (ui->text1PushButton,SIGNAL(clicked()),SLOT(slotText1()));
  connect (ui->text2PushButton,SIGNAL(clicked()),SLOT(slotText2()));
  connect (ui->text3PushButton,SIGNAL(clicked()),SLOT(slotText3()));
  connect (ui->text4PushButton,SIGNAL(clicked()),SLOT(slotText4()));
  readSettings();
  txt="";
}

waterfallForm::~waterfallForm()
{
  writeSettings();
  delete ui;
}


void waterfallForm::accept()
{
  writeSettings();
  done(QDialog::Accepted);
}

void waterfallForm::slotText1()
{
  getParams();
  txt=txt1;
  accept();
}

void waterfallForm::slotText2()
{
  getParams();
  txt=txt2;
  accept();
}

void waterfallForm::slotText3()
{
  getParams();
  txt=txt3;
  accept();
}

void waterfallForm::slotText4()
{
  getParams();
  txt=txt4;
  accept();
}

void waterfallForm::getParams()
{
  txt1=ui->wfText1->toPlainText();
  txt2=ui->wfText2->toPlainText();
  txt3=ui->wfText3->toPlainText();
  txt4=ui->wfText4->toPlainText();
}

void waterfallForm::setParams()
{
  ui->wfText1->setPlainText(txt1);
  ui->wfText2->setPlainText(txt2);
  ui->wfText3->setPlainText(txt3);
  ui->wfText4->setPlainText(txt4);
}

void waterfallForm::readSettings()
{
  QFont ft;
  QSettings qSettings;
  qSettings.beginGroup("Waterfall");
  txt1=qSettings.value("text1","").toString();
  txt2=qSettings.value("text2","").toString();
  txt3=qSettings.value("text3","").toString();
  txt4=qSettings.value("text4","").toString();
  qSettings.endGroup();
  setParams();
}

void waterfallForm::writeSettings()
{
  getParams();
  QSettings qSettings;
  qSettings.beginGroup("Waterfall");
  qSettings.setValue("text1",txt1);
  qSettings.setValue("text2",txt2);
  qSettings.setValue("text3",txt3);
  qSettings.setValue("text4",txt4);
  qSettings.endGroup();
}

