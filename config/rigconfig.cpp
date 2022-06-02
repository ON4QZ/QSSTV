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

#include "rigconfig.h"
#include "ui_rigconfig.h"
#include "appglobal.h"
#include "configparams.h"
#include "supportfunctions.h"
#include "rigcontrol.h"
#include <QSettings>
#include <QMessageBox>


rigConfig::rigConfig(QWidget *parent) : baseConfig(parent),  ui(new Ui::rigConfig)
{
  ui->setupUi(this);
  connect(ui->enableCATCheckBox,SIGNAL(clicked()),SLOT(slotEnableCAT()));
  connect(ui->enablePTTCheckBox,SIGNAL(clicked()),SLOT(slotEnablePTT()));
  connect(ui->enableXMLRPCCheckBox,SIGNAL(clicked()),SLOT(slotEnableXMLRPC()));
  connect(ui->restartPushButton,SIGNAL(clicked()),SLOT(slotRestart()));
  connect(ui->RTSCheckBox,SIGNAL(clicked()),SLOT(slotCheckPTT0()));
  connect(ui->DTRCheckBox,SIGNAL(clicked()),SLOT(slotCheckPTT1()));
  connect(ui->nRTSCheckBox,SIGNAL(clicked()),SLOT(slotCheckPTT2()));
  connect(ui->nDTRCheckBox,SIGNAL(clicked()),SLOT(slotCheckPTT3()));
  rigController=NULL;
  cp=NULL;
}


rigConfig::~rigConfig()
{
  delete ui;
}

void rigConfig::attachRigController(rigControl *rigCtrl)
{
  rigController=rigCtrl;
}

void rigConfig::readSettings()
{
  cp=rigController->params();
  QSettings qSettings;
  qSettings.beginGroup(cp->configLabel);
  cp->serialPort=qSettings.value("serialPort","/dev/ttyS0").toString();
  cp->radioModel=qSettings.value("radioModel","dummy").toString();
  cp->civAddress=qSettings.value("civAddress","").toString();
  cp->baudrate=qSettings.value("baudrate",9600).toInt();
  cp->parity=qSettings.value("parity","None").toString();
  cp->stopbits=qSettings.value("stopbits",1).toInt();
  cp->databits=qSettings.value("databits",8).toInt();
  cp->handshake=qSettings.value("handshake","None").toString();
  cp->enableCAT=qSettings.value("enableCAT",0).toBool();
  cp->enableSerialPTT=qSettings.value("enableSerialPTT",0).toBool();
  cp->pttSerialPort=qSettings.value("pttSerialPort","/dev/ttyS0").toString();
  cp->activeRTS=qSettings.value("activeRTS",1).toBool();
  cp->activeDTR=qSettings.value("activeDTR",0).toBool();
  cp->nactiveRTS=qSettings.value("nactiveRTS",1).toBool();
  cp->nactiveDTR=qSettings.value("nactiveDTR",0).toBool();
  cp->enableXMLRPC=qSettings.value("enableXMLRPC",0).toBool();
  cp->XMLRPCPort=qSettings.value("XMLRPCPort","7362").toInt();
  cp->txOnDelay=qSettings.value("txOnDelay",0.0).toDouble();
  cp->pttType=(ptt_type_t)qSettings.value("pttType",(int)RIG_PTT_RIG).toInt();
  qSettings.endGroup();
  setParams();
}

void rigConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup(cp->configLabel);
  qSettings.setValue("serialPort",cp->serialPort);
  qSettings.setValue("radioModel",cp->radioModel);
  qSettings.setValue("civAddress",cp->civAddress);
  qSettings.setValue("baudrate",cp->baudrate);
  qSettings.setValue("parity",cp->parity);
  qSettings.setValue("stopbits",cp->stopbits);
  qSettings.setValue("databits",cp->databits);
  qSettings.setValue("handshake",cp->handshake);
  qSettings.setValue("enableCAT",cp->enableCAT);
  qSettings.setValue("enableSerialPTT",cp->enableSerialPTT);
  qSettings.setValue("pttSerialPort",cp->pttSerialPort);
  qSettings.setValue("activeRTS",cp->activeRTS);
  qSettings.setValue("activeDTR",cp->activeDTR);
  qSettings.setValue("nactiveRTS",cp->nactiveRTS);
  qSettings.setValue("nactiveDTR",cp->nactiveDTR);
  qSettings.setValue("pttType",(int) cp->pttType);
  qSettings.setValue("enableXMLRPC",cp->enableXMLRPC);
  qSettings.setValue("XMLRPCPort",cp->XMLRPCPort);
  qSettings.setValue("txOnDelay",cp->txOnDelay);
  qSettings.endGroup();
}

void rigConfig::getParams()
{
  scatParams *cpCopy=new scatParams;
  *cpCopy=*cp;
  getValue(cp->serialPort,ui->serialPortLineEdit);
  if(ui->radioModelComboBox->count()!=0) getValue(cp->radioModel,ui->radioModelComboBox);
  getValue(cp->civAddress,ui->civAddressLineEdit);
  getValue(cp->baudrate,ui->baudrateComboBox);
  getValue(cp->parity,ui->parityComboBox);
  getValue(cp->stopbits,ui->stopbitsComboBox);
  getValue(cp->databits,ui->databitsComboBox);
  getValue(cp->handshake,ui->handshakeComboBox);
  getValue(cp->enableCAT,ui->enableCATCheckBox);
  getValue(cp->enableSerialPTT,ui->enablePTTCheckBox);
  getValue(cp->pttSerialPort,ui->pttSerialPortLineEdit);
  getValue(cp->activeRTS,ui->RTSCheckBox);
  getValue(cp->activeDTR,ui->DTRCheckBox);
  getValue(cp->nactiveRTS,ui->nRTSCheckBox);
  getValue(cp->nactiveDTR,ui->nDTRCheckBox);
  if(ui->catVoiceRadioButton->isChecked()) cp->pttType=RIG_PTT_RIG;
  if(ui->catDataRadioButton->isChecked()) cp->pttType=RIG_PTT_RIG_MICDATA;
  if(ui->rtsRadioButton->isChecked()) cp->pttType=RIG_PTT_SERIAL_RTS;
  if(ui->dtrRadioButton->isChecked()) cp->pttType=RIG_PTT_SERIAL_DTR;
  getValue(cp->txOnDelay,ui->txOnDelayDoubleSpinBox);
  getValue(cp->enableXMLRPC,ui->enableXMLRPCCheckBox);
  getValue(cp->XMLRPCPort,ui->XMLRPCPortLineEdit);
  changed=false;
  if( cpCopy->serialPort!=cp->serialPort
      || cpCopy->radioModel!=cp->radioModel
      || cpCopy->civAddress!=cp->civAddress
      || cpCopy->baudrate!=cp->baudrate
      || cpCopy->stopbits!=cp->stopbits
      || cpCopy->databits!=cp->databits
      || cpCopy->handshake!=cp->handshake
      || cpCopy->enableCAT!=cp->enableCAT
      || cpCopy->enableSerialPTT!=cp->enableSerialPTT
      || cpCopy->pttSerialPort!=cp->pttSerialPort
      || cpCopy->activeRTS!=cp->activeRTS
      || cpCopy->activeDTR!=cp->activeDTR
      || cpCopy->nactiveRTS!=cp->nactiveRTS
      || cpCopy->nactiveDTR!=cp->nactiveDTR
      || cpCopy->pttType!=cp->pttType
      || cpCopy->enableXMLRPC!=cp->enableXMLRPC
      || cpCopy->XMLRPCPort!=cp->XMLRPCPort
      )
    changed=true;
  delete cpCopy;
}


void rigConfig::setParams()
{
  if(rigController->getRadioList(ui->radioModelComboBox)) setValue(cp->radioModel,ui->radioModelComboBox);
  setValue(cp->serialPort,ui->serialPortLineEdit);
  setValue(cp->civAddress,ui->civAddressLineEdit);
  setValue(cp->baudrate,ui->baudrateComboBox);
  setValue(cp->parity,ui->parityComboBox);
  setValue(cp->stopbits,ui->stopbitsComboBox);
  setValue(cp->databits,ui->databitsComboBox);
  setValue(cp->handshake,ui->handshakeComboBox);
  setValue(cp->enableCAT,ui->enableCATCheckBox);
  setValue(cp->enableSerialPTT,ui->enablePTTCheckBox);
  setValue(cp->pttSerialPort,ui->pttSerialPortLineEdit);
  if(cp->activeRTS) cp->nactiveRTS=false;
  if(cp->activeDTR) cp->nactiveDTR=false;

  setValue(cp->activeRTS,ui->RTSCheckBox);
  setValue(cp->activeDTR,ui->DTRCheckBox);
  setValue(cp->nactiveRTS,ui->nRTSCheckBox);
  setValue(cp->nactiveDTR,ui->nDTRCheckBox);
  switch(cp->pttType)
    {
    case RIG_PTT_RIG:
      setValue(true,ui->catVoiceRadioButton);
    break;
    case RIG_PTT_RIG_MICDATA:
      setValue(true,ui->catDataRadioButton);
    break;
    case RIG_PTT_SERIAL_RTS:
      setValue(true,ui->rtsRadioButton);
    break;
    case RIG_PTT_SERIAL_DTR:
      setValue(true,ui->dtrRadioButton);
    break;
    default:
      setValue(true,ui->catVoiceRadioButton);
    break;
    }
  if(cp->enableCAT && cp->enableSerialPTT)
    {
      if(cp->serialPort==cp->pttSerialPort)
        {
          cp->enableSerialPTT=false;
        }
    }
  setValue(cp->enableSerialPTT,ui->enablePTTCheckBox);
  setValue(cp->txOnDelay,ui->txOnDelayDoubleSpinBox);
  setValue(cp->enableXMLRPC,ui->enableXMLRPCCheckBox);
  setValue(cp->XMLRPCPort,ui->XMLRPCPortLineEdit);
}



void rigConfig::slotEnableCAT()
{
  if(ui->enableCATCheckBox->isChecked() && ui->enablePTTCheckBox->isChecked())
    {
      if(ui->pttSerialPortLineEdit->text()==ui->serialPortLineEdit->text())
        {
          QMessageBox::critical(this,"Configuration error",
                                "The PTT serialport must be different from the CAT serial port if both are enabled");
          ui->enablePTTCheckBox->setChecked(false);
        }

    }
  if(ui->enableCATCheckBox->isChecked())
    {
      ui->enableXMLRPCCheckBox->setChecked(false);
      rigController->init();
    }
  else
    {
      rigController->disable();
    }
  getParams();
}



void rigConfig::slotEnablePTT()
{
  if(ui->enableCATCheckBox->isChecked() && ui->enablePTTCheckBox->isChecked())
    {
      if(ui->pttSerialPortLineEdit->text()==ui->serialPortLineEdit->text())
        {
          QMessageBox::critical(this,"Configuration error",
                                "The PTT serialport must be different from the CAT serial port if both are enabled");
          ui->enablePTTCheckBox->setChecked(false);
          return;
        }

    }
  if(ui->enablePTTCheckBox->isChecked())
    {
      ui->enableXMLRPCCheckBox->setChecked(false);
    }
  getParams();
}

void rigConfig::slotEnableXMLRPC()
{
  ui->enableCATCheckBox->setChecked(false);
  ui->enablePTTCheckBox->setChecked(false);
}

void rigConfig::slotRestart()
{
  getParams();
  if(ui->enableCATCheckBox->isChecked())
    {
      if(rigController->init())
        {
          ui->restartPushButton->setStyleSheet("background-color: green");
        }
      else
        {
          ui->restartPushButton->setStyleSheet("background-color: red");
        }
    }
}


void rigConfig::slotCheckPTT0()
{
  checkPTT(0,ui->RTSCheckBox->isChecked());
}
void rigConfig::slotCheckPTT1()
{checkPTT(1,ui->DTRCheckBox->isChecked());
}
void rigConfig::slotCheckPTT2()
{
  checkPTT(2,ui->nRTSCheckBox->isChecked());
}
void rigConfig::slotCheckPTT3()
{
  checkPTT(3,ui->nDTRCheckBox->isChecked());
}

void rigConfig::checkPTT(int p,bool b)
{
  if(!b) return;
  switch (p)
    {
    case 0:
      setValue(false,ui->nRTSCheckBox);
    break;
    case 1:
      setValue(false,ui->nDTRCheckBox);
    break;
    case 2:
      setValue(false,ui->RTSCheckBox);
    break;
    case 3:
      setValue(false,ui->DTRCheckBox);
    break;
    }
}

