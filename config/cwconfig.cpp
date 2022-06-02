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

#include "cwconfig.h"
#include "ui_cwconfig.h"

QString cwText;
int cwTone;
int cwWPM;
//bool enableCW;

cwConfig::cwConfig(QWidget *parent) :  baseConfig(parent), ui(new Ui::cwConfig)
{
  ui->setupUi(this);
}

cwConfig::~cwConfig()
{
  delete ui;
}

void cwConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("CW");
  cwText=qSettings.value("cwtext","").toString();
  cwTone=qSettings.value("cwtone",800).toInt();
  cwWPM=qSettings.value("cwWPM",12).toInt();
//  enableCW=qSettings.value("enableCW",false).toBool();
  qSettings.endGroup();
  setParams();
}

void cwConfig::writeSettings()
{
  QSettings qSettings;
  getParams();

  qSettings.beginGroup("CW");
  qSettings.setValue("cwtext",cwText);
  qSettings.setValue("cwtone",cwTone);
  qSettings.setValue("cwWPM",cwWPM);
//  qSettings.setValue("enableCW",enableCW);
  qSettings.endGroup();
}

void cwConfig::getParams()
{
  QString cwTextCopy=cwText;
  int cwToneCopy=cwTone;
  int cwWPMCopy=cwWPM;
  getValue(cwText,ui->cwTextLineEdit);
  getValue(cwTone,ui->cwToneSpinBox);
  getValue(cwWPM,ui->cwWPMSpinBox);
//  getValue(enableCW,ui->enableCWCheckBox);
  changed=false;
  if(cwTextCopy!=cwText
     || cwToneCopy!=cwTone
     || cwWPMCopy!=cwWPM)
    changed=true;
}

void cwConfig::setParams()
{
  setValue(cwText,ui->cwTextLineEdit);
  setValue(cwTone,ui->cwToneSpinBox);
  setValue(cwWPM,ui->cwWPMSpinBox);
//  setValue(enableCW,ui->enableCWCheckBox);

}
