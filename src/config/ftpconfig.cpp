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

#include "ftpconfig.h"
#include "appglobal.h"
#include "ui_ftpconfig.h"
#include "ftpfunctions.h"

#include <QImageWriter>
#include <QMessageBox>
#include <QDebug>


bool enableFTP;
int ftpPort;
QString ftpRemoteHost;
QString ftpRemoteSSTVDirectory;
QString ftpRemoteDRMDirectory;
QString ftpLogin;
QString ftpPassword;
QString ftpDefaultImageFormat;
eftpSaveFormat ftpSaveFormat;
int ftpNumImages;
bool addExtension;

ftpConfig::ftpConfig(QWidget *parent) :  baseConfig (parent),  ui(new Ui::ftpConfig)
{
  ui->setupUi(this);
  foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
      QString text = tr("%1").arg(QString(format));
      ui->ftpDefaultImageFormatComboBox->addItem(text);
    }
  connect(ui->testFTPPushButton,SIGNAL(clicked()),SLOT(slotTestFTPPushButton()));
}

ftpConfig::~ftpConfig()
{
  delete ui;
}

void ftpConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("FTPCONFIG");
  enableFTP=qSettings.value("enableFTP",false).toBool();
  ftpPort=qSettings.value("ftpPort",21).toInt();
  ftpRemoteHost=qSettings.value("ftpRemoteHost","").toString();
  ftpRemoteSSTVDirectory=qSettings.value("ftpRemoteSSTVDirectory","").toString();
  ftpRemoteDRMDirectory=qSettings.value("ftpRemoteDRMDirectory","").toString();
  ftpLogin=qSettings.value("ftpLogin","").toString();
  ftpPassword=qSettings.value("ftpPassword","").toString();
  ftpDefaultImageFormat=qSettings.value("ftpDefaultImageFormat","png").toString();
  ftpSaveFormat=(eftpSaveFormat)qSettings.value("ftpSaveFormat",0).toInt();
  ftpNumImages=qSettings.value("ftpNumImages",30).toInt();
  addExtension=qSettings.value("addExtension",false).toBool();
  qSettings.endGroup();

  setParams();
}

void ftpConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("FTPCONFIG");
  qSettings.setValue("enableFTP",enableFTP);
  qSettings.setValue("ftpPort",ftpPort);
  qSettings.setValue("ftpRemoteHost",ftpRemoteHost);
  qSettings.setValue("ftpRemoteSSTVDirectory",ftpRemoteSSTVDirectory);
  qSettings.setValue("ftpRemoteDRMDirectory",ftpRemoteDRMDirectory);
  qSettings.setValue("ftpLogin",ftpLogin);
  qSettings.setValue("ftpPassword",ftpPassword);
  qSettings.setValue("ftpDefaultImageFormat",ftpDefaultImageFormat);
  qSettings.setValue("ftpSaveFormat",(int)ftpSaveFormat);
  qSettings.setValue("ftpNumImages",ftpNumImages);
  qSettings.setValue("addExtension",addExtension);

  qSettings.endGroup();
}

void ftpConfig::getParams()
{
  bool enableFTPCopy=enableFTP;
  int ftpPortCopy=ftpPort;
  QString ftpRemoteHostCopy=ftpRemoteHost;
  QString ftpRemoteSSTVDirectoryCopy=ftpRemoteSSTVDirectory;
  QString ftpRemoteDRMDirectoryCopy=ftpRemoteDRMDirectory;
  QString ftpLoginCopy=ftpLogin;
  QString ftpPasswordCopy=ftpPassword;
  QString ftpDefaultImageFormatCopy=ftpDefaultImageFormat;
  eftpSaveFormat ftpSaveFormatCopy=ftpSaveFormat;


  getValue(enableFTP,ui->enableFTPCheckBox);
  getValue(ftpPort,ui->ftpPortSpinBox);
  getValue(ftpRemoteHost,ui->remoteHostLineEdit);
  getValue(ftpNumImages,ui->ftpNumImagesSpinBox);
  getValue(ftpRemoteSSTVDirectory,ui->remoteSSTVDirectoryLineEdit);
  getValue(ftpRemoteDRMDirectory,ui->remoteDRMDirectoryLineEdit);
  getValue(ftpLogin,ui->ftpLoginLineEdit);
  getValue(ftpPassword,ui->ftpPasswordLineEdit);
  getValue(ftpDefaultImageFormat,ui->ftpDefaultImageFormatComboBox);
  getValue(addExtension,ui->addExtensionCheckBox);
  if(ui->imageRadioButton->isChecked())
    {
      ftpSaveFormat=FTPIMAGESEQUENCE;
    }
  else
    {
      ftpSaveFormat=FTPFILENAME;
    }
  changed=false;
  if(enableFTPCopy!=enableFTP
     || ftpPortCopy!=ftpPort
     || ftpRemoteHostCopy!=ftpRemoteHost
     || ftpRemoteSSTVDirectoryCopy!=ftpRemoteSSTVDirectory
     || ftpRemoteDRMDirectoryCopy!=ftpRemoteDRMDirectory
     || ftpLoginCopy!=ftpLogin
     || ftpPasswordCopy!=ftpPassword
     || ftpDefaultImageFormatCopy!=ftpDefaultImageFormat
     || ftpSaveFormatCopy!=ftpSaveFormat)
    changed=true;
}

void ftpConfig::setParams()
{
  setValue(enableFTP,ui->enableFTPCheckBox);
  setValue(ftpPort,ui->ftpPortSpinBox);
  setValue(ftpRemoteHost,ui->remoteHostLineEdit);
  setValue(ftpNumImages,ui->ftpNumImagesSpinBox);
  setValue(ftpRemoteSSTVDirectory,ui->remoteSSTVDirectoryLineEdit);
  setValue(ftpRemoteDRMDirectory,ui->remoteDRMDirectoryLineEdit);
  setValue(ftpLogin,ui->ftpLoginLineEdit);
  setValue(ftpPassword,ui->ftpPasswordLineEdit);
  if(ftpSaveFormat==FTPIMAGESEQUENCE)
    {
      ui->imageRadioButton->setChecked(true);
    }
  else
    {
      ui->filenameRadioButton->setChecked(true);
    }
  setValue(ftpDefaultImageFormat,ui->ftpDefaultImageFormatComboBox);
  setValue(addExtension,ui->addExtensionCheckBox);
}


void ftpConfig::slotTestFTPPushButton()
{
  eftpError result;
  ftpFunctions ff;
  QString resultSSTVStr;
  QString resultDRMStr;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ui->testFTPPushButton->setDisabled(true);
  getParams();
  ff.test("Ftp Test sstvdir",ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,ftpRemoteSSTVDirectory,true);
  result=ff.getLastErrorStr(resultSSTVStr);


  if(result==FTPOK)
    {
//      qDebug() << "secondTest()";
      ff.test("Ftp Test drmdir",ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,ftpRemoteDRMDirectory,false);
      ff.getLastErrorStr(resultDRMStr);
    }
  QApplication::restoreOverrideCursor();
  ui->testFTPPushButton->setDisabled(false);
  QMessageBox::information(this,"FTP Site Test",resultSSTVStr+"\n"+resultDRMStr,QMessageBox::Ok);
}

