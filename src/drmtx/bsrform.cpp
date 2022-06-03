#include "bsrform.h"
#include "ui_bsrform.h"
#include "drmstatusframe.h"
#include "drm.h"

bsrForm::bsrForm(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::bsrForm)
{
  ui->setupUi(this);
  connect(ui->bsrComboBox,SIGNAL(currentIndexChanged(int)),this, SLOT(slotBSRSelection(int)));
  connect(ui->cancelPushButton,SIGNAL(clicked()),this,SLOT(slotCanceled()));
  connect(ui->easypalPushButton,SIGNAL(clicked()),this,SLOT(slotEasypal()));
  connect(ui->compatiblePushButton,SIGNAL(clicked()),this,SLOT(slotCompatible()));

}

bsrForm::~bsrForm()
{
  delete ui;
}

void bsrForm::init()
{
  int i;
  bsrPtr=srcDecoder->getBSR();
  if(bsrPtr->count()==0)
    {
      ui->infoTextEdit->clear();
      ui->infoTextEdit->appendPlainText("No BSR available");
      return;
    }
  for(i=bsrPtr->count()-1;i>=0;i--) //latest first
    {
      ui->bsrComboBox->addItem(bsrPtr->at(i).tbPtr->fileName);
    }
  slotBSRSelection(0);
}

bool bsrForm::hasBSR()
{
  return (bsrPtr->count()>0);
}

QByteArray *bsrForm::getBA(bool compat)
{
  int i;
  i=bsrPtr->count()-1-ui->bsrComboBox->currentIndex();
  if(srcDecoder->storeBSR(bsrPtr->at(i).tbPtr,compat))
    {
      drmParams.robMode=bsrPtr->at(i).tbPtr->robMode;
      drmParams.interleaver=bsrPtr->at(i).tbPtr->interLeaver;
      drmParams.qam=bsrPtr->at(i).tbPtr->mscMode;
      drmParams.protection=bsrPtr->at(i).tbPtr->mpx;
      drmParams.bandwith=bsrPtr->at(i).tbPtr->spectrum;
      return(&bsrPtr->at(i).tbPtr->baBSR);
    }
  return NULL;
}

 void bsrForm::slotBSRSelection(int idx)
{
  int i;
  transportBlock *tbPtr;
  i=bsrPtr->count()-1-idx;
  ui->infoTextEdit->clear();
  tbPtr=bsrPtr->at(i).tbPtr;
  ui->infoTextEdit->appendPlainText(tbPtr->callsign);
  ui->infoTextEdit->appendPlainText("Segments received: "+QString::number(tbPtr->segmentsReceived));
  ui->infoTextEdit->appendPlainText("Total Segments: "+QString::number(tbPtr->totalSegments));
  ui->infoTextEdit->appendPlainText(modeToString(tbPtr->modeCode));
}

void bsrForm::slotCanceled()
{
  done(CANCEL);
}



void bsrForm::slotEasypal()
{
  done(EASYPAL);
}

void bsrForm::slotCompatible()
{
  done (COMPAT);
}
