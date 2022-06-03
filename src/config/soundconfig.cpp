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

#include "soundconfig.h"
#include "ui_soundconfig.h"
#include "configparams.h"
#include "supportfunctions.h"
#include "soundalsa.h"

#include <QSettings>

#include <math.h>

int samplingrate;
double rxClock;
double txClock;
bool pulseSelected;
bool alsaSelected;
bool swapChannel;
bool pttToneOtherChannel;
QString inputAudioDevice;
QString outputAudioDevice;
soundBase::edataSrc soundRoutingInput;
soundBase::edataDst soundRoutingOutput;

quint32 recordingSize;



soundConfig::soundConfig(QWidget *parent) :  baseConfig(parent), ui(new Ui::soundConfig)
{
  QStringList inputPCMList, outputPCMList;
  ui->setupUi(this);
  getCardList(inputPCMList, outputPCMList);
  ui->inputPCMNameComboBox->addItems(inputPCMList);
  ui->outputPCMNameComboBox->addItems(outputPCMList);
}


soundConfig::~soundConfig()
{
  delete ui;
}

void soundConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("SOUND");
  rxClock=qSettings.value("rxclock",BASESAMPLERATE).toDouble();
  txClock=qSettings.value("txclock",BASESAMPLERATE).toDouble();
  if(fabs(1-rxClock/BASESAMPLERATE)>0.002) rxClock=BASESAMPLERATE;
  if(fabs(1-txClock/BASESAMPLERATE)>0.002) txClock=BASESAMPLERATE;
  samplingrate=BASESAMPLERATE;
  inputAudioDevice=qSettings.value("inputAudioDevice","default").toString();
  outputAudioDevice=qSettings.value("outputAudioDevice","default").toString();
  alsaSelected=qSettings.value("alsaSelected",false).toBool();
  pulseSelected=qSettings.value("pulseSelected",false).toBool();
  swapChannel=qSettings.value("swapChannel",false).toBool();
  pttToneOtherChannel=qSettings.value("pttToneOtherChannel",false).toBool();
  soundRoutingInput=  (soundBase::edataSrc)qSettings.value("soundRoutingInput",  0 ).toInt();
  soundRoutingOutput= (soundBase::edataDst)qSettings.value("soundRoutingOutput", 0 ).toInt();
  recordingSize= qSettings.value("recordingSize", 100 ).toInt();
  qSettings.endGroup();
  setParams();
}

void soundConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("SOUND");
  qSettings.setValue("rxclock",rxClock);
  qSettings.setValue("txclock",txClock);
  qSettings.setValue("inputAudioDevice",inputAudioDevice);
  qSettings.setValue("outputAudioDevice",outputAudioDevice);
  qSettings.setValue("alsaSelected",alsaSelected);
  qSettings.setValue("pulseSelected",pulseSelected);
  qSettings.setValue("swapChannel",swapChannel);
  qSettings.setValue("pttToneOtherChannel",pttToneOtherChannel);
  qSettings.setValue ("soundRoutingInput", soundRoutingInput );
  qSettings.setValue ("soundRoutingOutput",soundRoutingOutput );
  qSettings.setValue ("recordingSize",recordingSize );
  qSettings.endGroup();
}


void soundConfig::setParams()
{
  setValue(rxClock,ui->inputClockLineEdit,9);
  setValue(txClock,ui->outputClockLineEdit,9);
  setValue(inputAudioDevice,ui->inputPCMNameComboBox);
  setValue(outputAudioDevice,ui->outputPCMNameComboBox);
  setValue(alsaSelected,ui->alsaRadioButton);
  setValue(pulseSelected,ui->pulseRadioButton);
  setValue(swapChannel,ui->swapChannelCheckBox);
  setValue(pttToneOtherChannel,ui->pttToneCheckBox);
  if(soundRoutingInput==soundBase::SNDINCARD) ui->inFromCard->setChecked(true);
  else if (soundRoutingInput==soundBase::SNDINFROMFILE) ui->inFromFile->setChecked(true);
  else ui->inRecordFromCard->setChecked(true);

  if(soundRoutingOutput==soundBase::SNDOUTCARD) ui->outToCard->setChecked(true);
  else ui->outRecord->setChecked(true);
  setValue(recordingSize,ui->mbSpinBox);
}

void soundConfig::getParams()
{
  QString inputAudioDeviceCopy=inputAudioDevice;
  QString  outputAudioDeviceCopy=outputAudioDevice;
  bool alsaSelectedCopy=alsaSelected;


  soundBase::edataSrc soundRoutingInputCopy=soundRoutingInput;
  soundBase::edataDst soundRoutingOutputCopy=soundRoutingOutput;

  getValue(rxClock,ui->inputClockLineEdit);
  getValue(txClock,ui->inputClockLineEdit);
  getValue(inputAudioDevice,ui->inputPCMNameComboBox);
  getValue(outputAudioDevice,ui->outputPCMNameComboBox);
  getValue(alsaSelected,ui->alsaRadioButton);
  getValue(pulseSelected,ui->pulseRadioButton);
  getValue(swapChannel,ui->swapChannelCheckBox);
  getValue(pttToneOtherChannel,ui->pttToneCheckBox);

  if (ui->inFromCard->isChecked()) soundRoutingInput=soundBase::SNDINCARD;
  else if(ui->inFromFile->isChecked()) soundRoutingInput=soundBase::SNDINFROMFILE;
  else soundRoutingInput=soundBase::SNDINCARDTOFILE;

  if (ui->outToCard->isChecked()) soundRoutingOutput=soundBase::SNDOUTCARD;
  else soundRoutingOutput=soundBase::SNDOUTTOFILE;
  getValue(recordingSize,ui->mbSpinBox);
  changed=false;
  if(inputAudioDeviceCopy!=inputAudioDevice
     || outputAudioDeviceCopy!=outputAudioDevice
     || soundRoutingInputCopy!=soundRoutingInput
     || soundRoutingOutputCopy!=soundRoutingOutput
     || alsaSelectedCopy!=alsaSelected)
  {
    changed=true;
  }
}
