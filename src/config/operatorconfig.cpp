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

#include "operatorconfig.h"
#include "ui_operatorconfig.h"

QString myCallsign;
QString myQth;
QString myLocator;
QString myLastname;
QString myFirstname;
QString lastReceivedCall;
bool    onlineStatusEnabled;
QString onlineStatusText;


operatorConfig::operatorConfig(QWidget *parent) :  baseConfig(parent), ui(new Ui::operatorConfig)
{
  ui->setupUi(this);
  QRegExp rx("^\\w*$");
  QValidator *validator = new QRegExpValidator(rx, this);
  ui->onlineStatusText->setValidator(validator);
}

operatorConfig::~operatorConfig()
{
  delete ui;
}

void operatorConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("PERSONAL");
  myCallsign=qSettings.value("callsign",QString("NOCALL")).toString();
  myQth=qSettings.value("qth",QString("NOWHERE")).toString();
  myLastname=qSettings.value("lastname",QString("NONAME")).toString();
  myFirstname=qSettings.value("firstname",QString("NOFIRSTNAME")).toString();
  myLocator=qSettings.value("locator",QString("NOLOCATOR")).toString();
  onlineStatusEnabled=qSettings.value("onlinestatusenabled",true).toBool();
  onlineStatusText=qSettings.value("onlinestatustext",QString("")).toString();
  qSettings.endGroup();
  setParams();
}

void operatorConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("PERSONAL");
  qSettings.setValue("callsign",myCallsign);
  qSettings.setValue("qth",myQth);
  qSettings.setValue("locator",myLocator);
  qSettings.setValue("lastname",myLastname);
  qSettings.setValue("firstname",myFirstname);
  qSettings.setValue("onlinestatusenabled",onlineStatusEnabled);
  qSettings.setValue("onlinestatustext",onlineStatusText);
  qSettings.endGroup();
}

void operatorConfig::getParams()
{
  QString myCallsignCopy=myCallsign;
  QString myQthCopy=myQth;
  QString myLocatorCopy= myLocator;
  QString myLastnameCopy=myLastname;
  QString myFirstnameCopy=myFirstname;
  QString onlineStatusTextCopy=onlineStatusText;
  bool    onlineStatusEnabledCopy=onlineStatusEnabled;

  getValue(myCallsign,ui->callsignLineEdit);
  getValue(myLastname,ui->lastnameLineEdit);
  getValue(myFirstname,ui->firstnameLineEdit);
  getValue(myQth,ui->qthLineEdit);
  getValue(myLocator,ui->locatorLineEdit);
  getValue(onlineStatusText,ui->onlineStatusText);





  getValue(onlineStatusEnabled,ui->onlineStatusCheckbox);

  changed=false;
  if( myCallsignCopy!=myCallsign
      || myQthCopy!=myQth
      || myLocatorCopy!= myLocator
      || myLastnameCopy!=myLastname
      || myFirstnameCopy!=myFirstname
      || onlineStatusEnabledCopy!=onlineStatusEnabled
      || onlineStatusTextCopy!=onlineStatusText)
    changed=true;
}

void operatorConfig::setParams()
{
  setValue(myCallsign,ui->callsignLineEdit);
  setValue(myLastname,ui->lastnameLineEdit);
  setValue(myFirstname,ui->firstnameLineEdit);
  setValue(myQth,ui->qthLineEdit);
  setValue(myLocator,ui->locatorLineEdit);
  setValue(onlineStatusEnabled,ui->onlineStatusCheckbox);
  setValue(onlineStatusText,ui->onlineStatusText);
}
