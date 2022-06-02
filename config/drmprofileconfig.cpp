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

#include "drmprofileconfig.h"
#include "ui_drmprofileconfig.h"
#include "supportfunctions.h"
#include "configparams.h"

sprofile drmPFArray[NUMBEROFPROFILES];

drmProfileConfig *drmProfileConfigPtr;


drmProfileConfig::drmProfileConfig(QWidget *parent) :  baseConfig(parent),  ui(new Ui::drmProfileConfig)
{
  ui->setupUi(this);
  readSettings();
}

drmProfileConfig::~drmProfileConfig()
{
    writeSettings();
  delete ui;
}


void drmProfileConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("DRMPROFILE" );
  drmPFArray[0].name=qSettings.value ("drmPF1Name","Profile 1").toString();
  drmPFArray[0].params.robMode=qSettings.value ("drmPF1Mode",0).toInt();
  drmPFArray[0].params.qam=qSettings.value("drmPF1QAM",0).toInt();
  drmPFArray[0].params.bandwith=qSettings.value("drmPF1Bandwidth",0).toInt();
  drmPFArray[0].params.protection=qSettings.value("drmPF1Protection",0).toInt();
  drmPFArray[0].params.interleaver=qSettings.value("drmPF1Interleave",0).toInt();
  drmPFArray[0].params.reedSolomon=qSettings.value("drmPF1ReedSolomon",0).toInt();

  drmPFArray[1].name=qSettings.value ("drmPF2Name","Profile 2").toString();
  drmPFArray[1].params.robMode=qSettings.value ("drmPF2Mode",0).toInt();
  drmPFArray[1].params.qam=qSettings.value("drmPF2QAM",0).toInt();
  drmPFArray[1].params.bandwith=qSettings.value("drmPF2Bandwidth",0).toInt();
  drmPFArray[1].params.protection=qSettings.value("drmPF2Protection",0).toInt();
  drmPFArray[1].params.interleaver=qSettings.value("drmPF2Interleave",0).toInt();
  drmPFArray[1].params.reedSolomon=qSettings.value("drmPF2ReedSolomon",0).toInt();

  drmPFArray[2].name=qSettings.value ("drmPF3Name","Profile 3").toString();
  drmPFArray[2].params.robMode=qSettings.value ("drmPF3Mode",0).toInt();
  drmPFArray[2].params.qam=qSettings.value("drmPF3QAM",0).toInt();
  drmPFArray[2].params.bandwith=qSettings.value("drmPF3Bandwidth",0).toInt();
  drmPFArray[2].params.protection=qSettings.value("drmPF3Protection",0).toInt();
  drmPFArray[2].params.interleaver=qSettings.value("drmPF3Interleave",0).toInt();
  drmPFArray[2].params.reedSolomon=qSettings.value("drmPF3ReedSolomon",0).toInt();
  qSettings.endGroup();
  setParams();

}

void drmProfileConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup ("DRMPROFILE" );
  qSettings.setValue ("drmPF1Name",drmPFArray[0].name);
  qSettings.setValue ("drmPF1Mode",drmPFArray[0].params.robMode);
  qSettings.setValue("drmPF1QAM",drmPFArray[0].params.qam);
  qSettings.setValue("drmPF1Bandwidth",drmPFArray[0].params.bandwith);
  qSettings.setValue("drmPF1Protection",drmPFArray[0].params.protection);
  qSettings.setValue("drmPF1Interleave",drmPFArray[0].params.interleaver);
  qSettings.setValue("drmPF1ReedSolomon",drmPFArray[0].params.reedSolomon);
  qSettings.setValue ("drmPF2Name",drmPFArray[1].name);
  qSettings.setValue ("drmPF2Mode",drmPFArray[1].params.robMode);
  qSettings.setValue("drmPF2QAM",drmPFArray[1].params.qam);
  qSettings.setValue("drmPF2Bandwidth",drmPFArray[1].params.bandwith);
  qSettings.setValue("drmPF2Protection",drmPFArray[1].params.protection);
  qSettings.setValue("drmPF2Interleave",drmPFArray[1].params.interleaver);
  qSettings.setValue("drmPF2ReedSolomon",drmPFArray[1].params.reedSolomon);

  qSettings.setValue ("drmPF3Name",drmPFArray[2].name);
  qSettings.setValue ("drmPF3Mode",drmPFArray[2].params.robMode);
  qSettings.setValue("drmPF3QAM",drmPFArray[2].params.qam);
  qSettings.setValue("drmPF3Bandwidth",drmPFArray[2].params.bandwith);
  qSettings.setValue("drmPF3Protection",drmPFArray[2].params.protection);
  qSettings.setValue("drmPF3Interleave",drmPFArray[2].params.interleaver);
  qSettings.setValue("drmPF3ReedSolomon",drmPFArray[2].params.reedSolomon);
  qSettings.endGroup();
}


void drmProfileConfig::getParams()
{
  sprofile drmPFArrayCopy[NUMBEROFPROFILES];
  drmPFArrayCopy[0]=drmPFArray[0];
  drmPFArrayCopy[1]=drmPFArray[1];
  drmPFArrayCopy[2]=drmPFArray[2];

  getValue(drmPFArray[0].name,ui->namePF1LineEdit);
  drmPFArray[0].params.callsign=myCallsign;
  getIndex(drmPFArray[0].params.robMode,ui->drmPF1ModeComboBox);
  getIndex(drmPFArray[0].params.qam,ui->drmPF1QAMComboBox);
  getIndex(drmPFArray[0].params.bandwith,ui->drmPF1BandwidthComboBox);
  getIndex(drmPFArray[0].params.protection,ui->drmPF1ProtectionComboBox);
  getIndex(drmPFArray[0].params.interleaver,ui->drmPF1InterleaveComboBox);
  getIndex(drmPFArray[0].params.reedSolomon,ui->drmPF1ReedSolomonComboBox);

  getValue(drmPFArray[1].name,ui->namePF2LineEdit);
  drmPFArray[1].params.callsign=myCallsign;
  getIndex(drmPFArray[1].params.robMode,ui->drmPF2ModeComboBox);
  getIndex(drmPFArray[1].params.qam,ui->drmPF2QAMComboBox);
  getIndex(drmPFArray[1].params.bandwith,ui->drmPF2BandwidthComboBox);
  getIndex(drmPFArray[1].params.protection,ui->drmPF2ProtectionComboBox);
  getIndex(drmPFArray[1].params.interleaver,ui->drmPF2InterleaveComboBox);
  getIndex(drmPFArray[1].params.reedSolomon,ui->drmPF2ReedSolomonComboBox);

  getValue(drmPFArray[2].name,ui->namePF3LineEdit);
  drmPFArray[2].params.callsign=myCallsign;
  getIndex(drmPFArray[2].params.robMode,ui->drmPF3ModeComboBox);
  getIndex(drmPFArray[2].params.qam,ui->drmPF3QAMComboBox);
  getIndex(drmPFArray[2].params.bandwith,ui->drmPF3BandwidthComboBox);
  getIndex(drmPFArray[2].params.protection,ui->drmPF3ProtectionComboBox);
  getIndex(drmPFArray[2].params.interleaver,ui->drmPF3InterleaveComboBox);
  getIndex(drmPFArray[2].params.reedSolomon,ui->drmPF3ReedSolomonComboBox);
  changed=false;
  if( diff(drmPFArrayCopy[0],drmPFArray[0])
      || diff(drmPFArrayCopy[1],drmPFArray[1])
      || diff(drmPFArrayCopy[2],drmPFArray[2]))
    changed=true;

}

bool drmProfileConfig::diff(sprofile a,sprofile b)
{
  return
      (a.name!=b.name
      || a.params.robMode!=b.params.robMode
      || a.params.qam!= b.params.qam
      || a.params.bandwith!=b.params.bandwith
      || a.params.protection!=b.params.protection
      || a.params.interleaver!=b.params.interleaver
      || a.params.reedSolomon!=b.params.reedSolomon);

}

void  drmProfileConfig::setParams()
{
  setValue(drmPFArray[0].name,ui->namePF1LineEdit);
  setIndex(drmPFArray[0].params.robMode,ui->drmPF1ModeComboBox);
  setIndex(drmPFArray[0].params.qam,ui->drmPF1QAMComboBox);
  setIndex(drmPFArray[0].params.bandwith,ui->drmPF1BandwidthComboBox);
  setIndex(drmPFArray[0].params.protection,ui->drmPF1ProtectionComboBox);
  setIndex(drmPFArray[0].params.interleaver,ui->drmPF1InterleaveComboBox);
  setIndex(drmPFArray[0].params.reedSolomon,ui->drmPF1ReedSolomonComboBox);

  setValue(drmPFArray[1].name,ui->namePF2LineEdit);
  setIndex(drmPFArray[1].params.robMode,ui->drmPF2ModeComboBox);
  setIndex(drmPFArray[1].params.qam,ui->drmPF2QAMComboBox);
  setIndex(drmPFArray[1].params.bandwith,ui->drmPF2BandwidthComboBox);
  setIndex(drmPFArray[1].params.protection,ui->drmPF2ProtectionComboBox);
  setIndex(drmPFArray[1].params.interleaver,ui->drmPF2InterleaveComboBox);
  setIndex(drmPFArray[1].params.reedSolomon,ui->drmPF2ReedSolomonComboBox);

  setValue(drmPFArray[2].name,ui->namePF3LineEdit);
  setIndex(drmPFArray[2].params.robMode,ui->drmPF3ModeComboBox);
  setIndex(drmPFArray[2].params.qam,ui->drmPF3QAMComboBox);
  setIndex(drmPFArray[2].params.bandwith,ui->drmPF3BandwidthComboBox);
  setIndex(drmPFArray[2].params.protection,ui->drmPF3ProtectionComboBox);
  setIndex(drmPFArray[2].params.interleaver,ui->drmPF3InterleaveComboBox);
  setIndex(drmPFArray[2].params.reedSolomon,ui->drmPF3ReedSolomonComboBox);
}


bool drmProfileConfig::getDRMParams(int idx,drmTxParams &d)
{
  if((idx<0)||(idx>=NUMBEROFPROFILES))
  {
      return false;
  }
  d=drmPFArray[idx].params;
  d.callsign=myCallsign;
  return true;
}

bool drmProfileConfig::getName(int idx, QString &n)
{

    if((idx<0)||(idx>=NUMBEROFPROFILES))
    {
        return false;
    }
    n=drmPFArray[idx].name;
    return true;

}
