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

#include "hybridconfig.h"
#include "ui_hybridconfig.h"
#include "ftpfunctions.h"
#include "hybridcrypt.h"

#include <QMessageBox>

bool enableHybridRx;
//bool enableSpecialServer;
int hybridFtpPort;
QString hybridFtpRemoteHost;
QString hybridFtpRemoteDirectory;
QString hybridFtpLogin;
QString hybridFtpPassword;
QString hybridFtpHybridFilesDirectory;

bool enableHybridNotify;
QString hybridNotifyDir;
QString onlineStatusDir;


hybridConfig::hybridConfig(QWidget *parent) :baseConfig(parent), ui(new Ui::hybridConfig)
{
  ui->setupUi(this);
  //  connect(testFTPPushButton,SIGNAL(clicked()),SLOT(slotTestFTPPushButton()));
  connect(ui->testHybridPushButton,SIGNAL(clicked()),SLOT(slotTestHybridPushButton()));
}

hybridConfig::~hybridConfig()
{
  delete ui;
}

void hybridConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("HYBRID");
  enableHybridRx=qSettings.value("enableHybridRx",true).toBool();
  hybridFtpPort=qSettings.value("hybridFtpPort",21).toInt();
  hybridFtpRemoteHost=qSettings.value("hybridFtpRemoteHost","").toString();
  hybridFtpRemoteDirectory=qSettings.value("hybridFtpRemoteDirectory","").toString();
  hybridFtpLogin=qSettings.value("hybridFtpLogin","").toString();
  hybridFtpPassword=qSettings.value("hybridFtpPassword","").toString();
  hybridFtpHybridFilesDirectory=qSettings.value("hybridFtpHybridFilesDirectory","HybridFiles1").toString();
  hybridNotifyDir=qSettings.value("hybridNotifyDirectory","RxOkNotifications1").toString();
  onlineStatusDir=qSettings.value("onlineStatusDirectory","OnlineCallsigns1").toString();
  enableHybridNotify=qSettings.value("enableHybridNotify",true).toBool();


  qSettings.endGroup();
  setParams();
}

void hybridConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("HYBRID");
  qSettings.setValue("enableHybridRx",enableHybridRx);
  qSettings.setValue("hybridFtpPort",hybridFtpPort);
  qSettings.setValue("hybridFtpRemoteHost",hybridFtpRemoteHost);
  qSettings.setValue("hybridFtpRemoteDirectory",hybridFtpRemoteDirectory);
  qSettings.setValue("hybridFtpLogin",hybridFtpLogin);
  qSettings.setValue("hybridFtpPassword",hybridFtpPassword);
  qSettings.setValue("hybridFtpHybridFilesDirectory",hybridFtpHybridFilesDirectory);
  qSettings.setValue("enableHybridNotify",enableHybridNotify);
  qSettings.setValue("hybridNotifyDir",hybridNotifyDir);
  qSettings.setValue("onlineStatusDir",onlineStatusDir);
  qSettings.endGroup();
}

void hybridConfig::getParams()
{
  bool enableHybridRxCopy=enableHybridRx;
  bool enableHybridNotifyCopy=enableHybridNotify;
  int hybridFtpPortCopy=hybridFtpPort;
  QString hybridFtpRemoteHostCopy=hybridFtpRemoteHost;
  QString hybridFtpRemoteDirectoryCopy=hybridFtpRemoteDirectory;
  QString hybridFtpLoginCopy=hybridFtpLogin;
  QString hybridFtpPasswordCopy=hybridFtpPassword;
  QString hybridNotifyDirCopy=hybridNotifyDir;
  QString onlineStatusDirCopy=onlineStatusDir;


  getValue(enableHybridRx,ui->enableHybridRxCheckBox);
  getValue(hybridFtpPort,ui->hybridFtpPortSpinBox);
  getValue(hybridFtpRemoteHost,ui->hybridRemoteHostLineEdit);
  getValue(hybridFtpRemoteDirectory,ui->hybridRemoteDirectoryLineEdit);
  getValue(hybridFtpLogin,ui->hybridFtpLoginLineEdit);
  getValue(hybridFtpPassword,ui->hybridFtpPasswordLineEdit);
  getValue(hybridFtpHybridFilesDirectory,ui->hybridFilesDirectoryLineEdit);
  getValue(enableHybridNotify,ui->enableHybridNotifyCheckBox);
  getValue(hybridNotifyDir,ui->hybridNotifyDirLineEdit);
  getValue(onlineStatusDir,ui->onlineStatusDirLineEdit);

  changed=false;
  if(  enableHybridRxCopy!=enableHybridRx
       //         || enableSpecialServerCopy!=enableSpecialServer
       || hybridFtpPortCopy!=hybridFtpPort
       || hybridFtpRemoteHostCopy!=hybridFtpRemoteHost
       || hybridFtpRemoteDirectoryCopy!=hybridFtpRemoteDirectory
       || hybridFtpLoginCopy!=hybridFtpLogin
       || hybridFtpPasswordCopy!=hybridFtpPassword
       || enableHybridNotifyCopy!=enableHybridNotify
       || hybridNotifyDirCopy!=hybridNotifyDir
       || onlineStatusDirCopy!=onlineStatusDir
       )
    changed=true;

}

void hybridConfig::setParams()
{
  setValue(enableHybridRx,ui->enableHybridRxCheckBox);
  setValue(hybridFtpPort,ui->hybridFtpPortSpinBox);
  setValue(hybridFtpRemoteHost,ui->hybridRemoteHostLineEdit);
  setValue(hybridFtpRemoteDirectory,ui->hybridRemoteDirectoryLineEdit);
  setValue(hybridFtpLogin,ui->hybridFtpLoginLineEdit);
  setValue(hybridFtpPassword,ui->hybridFtpPasswordLineEdit);
  setValue(hybridFtpHybridFilesDirectory,ui->hybridFilesDirectoryLineEdit);
  setValue(enableHybridNotify,ui->enableHybridNotifyCheckBox);
  setValue(hybridNotifyDir,ui->hybridNotifyDirLineEdit);
  setValue(onlineStatusDir,ui->onlineStatusDirLineEdit);
}


void hybridConfig::slotTestHybridPushButton()
{
  ftpFunctions ff;
  QString resultStr1;
  QString resultStr2;
  QString resultStr3;


  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui->testHybridPushButton->setDisabled(true);
  getParams();
  ff.test("Hybrid Ftp Test",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin,hybridFtpPassword,hybridFtpRemoteDirectory+"/"+hybridFtpHybridFilesDirectory,true);
  ff.getLastErrorStr(resultStr1);

  ff.test("Notify Ftp Test",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin,hybridFtpPassword,hybridFtpRemoteDirectory+"/"+hybridNotifyDir,false);
  ff.getLastErrorStr(resultStr2);
  ff.test("OnlineStus Ftp Test",hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin,hybridFtpPassword,hybridFtpRemoteDirectory+"/"+onlineStatusDir,false);
  ff.getLastErrorStr(resultStr3);

  QApplication::restoreOverrideCursor();
  ui->testHybridPushButton->setDisabled(false);
  QMessageBox::information(this,"FTP Site Test",QString("%1\n%2\n%3").arg(resultStr1).arg(resultStr2).arg(resultStr3),QMessageBox::Ok);



}
