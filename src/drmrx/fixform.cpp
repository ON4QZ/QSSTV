#include "fixform.h"
#include "ui_fixform.h"
#include "drmstatusframe.h"
#include "appglobal.h"
#include "drm.h"
#include "configparams.h"


fixForm::fixForm(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::fixForm)
{
  ui->setupUi(this);
}

fixForm::~fixForm()
{
  delete ui;
}

void fixForm::setInfoInternal(int mode, QString fileName, int missing, QByteArray *ba)
{
  common(mode,fileName,missing);
//  ui->infoTextEdit->appendPlainText("BSR for a picture you send");
  ui->previewWidget->openImage(ba);
}

//void fixForm::setInfoExternal(int mode, QString fileName, int missing)
//{
//  common(mode,fileName,missing);
//  ui->infoTextEdit->appendPlainText("BSR for a picture you received");
//  ui->previewWidget->openImage(fileName,false,false,false);
//}

void fixForm::common(int mode,QString fileName,int missing)
{
  ui->filenameLineEdit->setText("Filename: "+fileName+"\n");
  if(callsignValid)
    {
      ui->infoTextEdit->appendPlainText("From: "+ QString(drmCallsign));
     }
  ui->infoTextEdit->appendPlainText(modeToString(mode));
  ui->infoTextEdit->appendPlainText(QString("Requested segments: %1").arg(missing));
}
