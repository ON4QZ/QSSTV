/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   http://users.telenet.be/on4qz                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "repeaterconfig.h"
#include "ui_repeaterconfig.h"
#include "directoriesconfig.h"
#include "drmparams.h"


bool repeaterEnabled;
int repeaterImageInterval;
esstvMode repeaterTxMode;
esstvMode repeaterIdleTxMode;
QString repeaterImage1;
QString repeaterImage2;
QString repeaterImage3;
QString repeaterImage4;
QString repeaterAcknowledge;
QString repeaterTemplate;
QString repeaterIdleTemplate;
int repeaterTxDelay;
int repeaterImageSize; // in kB

repeaterConfig::repeaterConfig(QWidget *parent) : baseConfig(parent),  ui(new Ui::repeaterConfig)
{
  int i;
  ui->setupUi(this);
  ui->repeaterTxModeComboBox->addItem("Same as RX");
  for(i=0;i<NUMSSTVMODES ;i++)
    {
      ui->repeaterTxModeComboBox->addItem(SSTVTable[i].name);
      ui->repeaterIdleTxModeComboBox->addItem(SSTVTable[i].name);
    }
  connect(ui->rp1BrowseButton,SIGNAL(clicked()),SLOT(slotRp1Browse()));
  connect(ui->rp2BrowseButton,SIGNAL(clicked()),SLOT(slotRp2Browse()));
  connect(ui->rp3BrowseButton,SIGNAL(clicked()),SLOT(slotRp3Browse()));
  connect(ui->rp4BrowseButton,SIGNAL(clicked()),SLOT(slotRp4Browse()));
  connect(ui->repeaterIdleTemplateBrowseButton,SIGNAL(clicked()),SLOT(slotRepeaterIdleTemplateBrowse()));
  connect(ui->repeaterTemplateBrowseButton,SIGNAL(clicked()),SLOT(slotRepeaterTemplateBrowse()));
  ui->txDRMImageSizeSpinBox->setMinimum(MINDRMSIZE/1000);
  ui->txDRMImageSizeSpinBox->setMaximum(MAXDRMSIZE/1000);

}

repeaterConfig::~repeaterConfig()
{
  delete ui;
}

void repeaterConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("REPEATER");
  repeaterImageInterval=qSettings.value("repeaterImageInterval",10).toInt();
  repeaterEnabled=qSettings.value("repeaterEnabled",false).toBool();
  repeaterTxMode=(esstvMode)qSettings.value("repeaterTxMode",0).toInt();
  repeaterIdleTxMode=(esstvMode)qSettings.value("repeaterIdleTxMode",0).toInt();
  repeaterImage1=qSettings.value("repeaterImage1","").toString();
  repeaterImage2=qSettings.value("repeaterImage2","").toString();
  repeaterImage3=qSettings.value("repeaterImage3","").toString();
  repeaterImage4=qSettings.value("repeaterImage4","").toString();
  repeaterAcknowledge=qSettings.value("repeaterAcknowledge","").toString();
  repeaterTemplate=qSettings.value("repeaterTemplate","").toString();
  repeaterIdleTemplate=qSettings.value("repeaterIdleTemplate","").toString();
  repeaterTxDelay=qSettings.value("repeaterTxDelay",5).toInt();
  repeaterImageSize=qSettings.value("repeaterImageSize",10).toInt();
  qSettings.endGroup();
  setParams();
}

void repeaterConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("REPEATER");
  qSettings.setValue("repeaterImageInterval",repeaterImageInterval);
  qSettings.setValue("repeaterEnabled",repeaterEnabled);
  qSettings.setValue("repeaterTxMode",repeaterTxMode);
  qSettings.setValue("repeaterIdleTxMode",repeaterIdleTxMode);
  qSettings.setValue("repeaterImage1",repeaterImage1);
  qSettings.setValue("repeaterImage2",repeaterImage2);
  qSettings.setValue("repeaterImage3",repeaterImage3);
  qSettings.setValue("repeaterImage4",repeaterImage4);
  qSettings.setValue("repeaterAcknowledge",repeaterAcknowledge);
  qSettings.setValue("repeaterTemplate",repeaterTemplate);
  qSettings.setValue("repeaterIdleTemplate",repeaterIdleTemplate);
  qSettings.setValue("repeaterTxDelay",repeaterTxDelay);
  qSettings.setValue("repeaterImageSize",repeaterImageSize);
  qSettings.endGroup();
}

void repeaterConfig::getParams()
{
  int temp;
  bool repeaterEnabledCopy=repeaterEnabled;
  int repeaterImageIntervalCopy=repeaterImageInterval;
  esstvMode repeaterTxModeCopy=repeaterTxMode;
  esstvMode repeaterIdleTxModeCopy=repeaterIdleTxMode;
  QString repeaterImage1Copy=repeaterImage1;
  QString repeaterImage2Copy=repeaterImage2;
  QString repeaterImage3Copy=repeaterImage3;
  QString repeaterImage4Copy=repeaterImage4;
  QString repeaterAcknowledgeCopy=repeaterAcknowledge;
  QString repeaterTemplateCopy=repeaterTemplate;
  QString repeaterIdleTemplateCopy=repeaterIdleTemplate;
  int repeaterTxDelayCopy=repeaterTxDelay;
  int repeaterImageSizeCopy=repeaterImageSize;
  getValue(repeaterImageInterval,ui->imageIntervalSpinBox);
  getValue(repeaterEnabled,ui->repeaterEnabledCheckBox);
  getIndex(temp,ui->repeaterTxModeComboBox);
  repeaterTxMode=(esstvMode)temp;
  getIndex(temp,ui->repeaterIdleTxModeComboBox);
  getValue(repeaterTxDelay,ui->txDelaySpinBox);
  getValue(repeaterImageSize,ui->txDRMImageSizeSpinBox);
  repeaterIdleTxMode=(esstvMode)temp;

  getValue(repeaterImage1,ui->repeaterImage1LineEdit);
  getValue(repeaterImage2,ui->repeaterImage2LineEdit);
  getValue(repeaterImage3,ui->repeaterImage3LineEdit);
  getValue(repeaterImage4,ui->repeaterImage4LineEdit);
  getValue(repeaterIdleTemplate,ui->repeaterIdleTemplateLineEdit);
  getValue(repeaterTemplate,ui->repeaterTemplateLineEdit);
  changed=false;
  if(  repeaterEnabledCopy!=repeaterEnabled
       || repeaterImageIntervalCopy!=repeaterImageInterval
       || repeaterTxModeCopy!=repeaterTxMode
       || repeaterIdleTxModeCopy!=repeaterIdleTxMode
       || repeaterImage1Copy!=repeaterImage1
       || repeaterImage2Copy!=repeaterImage2
       || repeaterImage3Copy!=repeaterImage3
       || repeaterImage4Copy!=repeaterImage4
       || repeaterAcknowledgeCopy!=repeaterAcknowledge
       || repeaterTemplateCopy!=repeaterTemplate
       || repeaterTxDelayCopy!=repeaterTxDelay
       || repeaterImageSizeCopy!=repeaterImageSize
       || repeaterIdleTemplateCopy!=repeaterIdleTemplate)
    changed=true;
}

void repeaterConfig::setParams()
{;
  setValue(repeaterImageInterval,ui->imageIntervalSpinBox);
  setValue(repeaterEnabled,ui->repeaterEnabledCheckBox);
  setIndex(repeaterTxMode,ui->repeaterTxModeComboBox);
  setIndex(repeaterIdleTxMode,ui->repeaterIdleTxModeComboBox);
  setValue(repeaterImage1,ui->repeaterImage1LineEdit);
  setValue(repeaterImage2,ui->repeaterImage2LineEdit);
  setValue(repeaterImage3,ui->repeaterImage3LineEdit);
  setValue(repeaterImage4,ui->repeaterImage4LineEdit);
  setValue(repeaterIdleTemplate,ui->repeaterIdleTemplateLineEdit);
  setValue(repeaterTemplate,ui->repeaterTemplateLineEdit);
  setValue(repeaterTxDelay,ui->txDelaySpinBox);
  setValue(repeaterImageSize,ui->txDRMImageSizeSpinBox);
}



void repeaterConfig::slotRp1Browse()
{
  browseGetFile(ui->repeaterImage1LineEdit,txStockImagesPath);
}

void repeaterConfig::slotRp2Browse()
{
  browseGetFile(ui->repeaterImage2LineEdit,txStockImagesPath);
}
void repeaterConfig::slotRp3Browse()
{
  browseGetFile(ui->repeaterImage3LineEdit,txStockImagesPath);
}
void repeaterConfig::slotRp4Browse()
{
  browseGetFile(ui->repeaterImage4LineEdit,txStockImagesPath);
}

void repeaterConfig::slotRepeaterIdleTemplateBrowse()
{
  browseGetFile(ui->repeaterIdleTemplateLineEdit ,templatesPath);
}
void repeaterConfig::slotRepeaterTemplateBrowse()
{
  browseGetFile(ui->repeaterTemplateLineEdit,templatesPath);
}


