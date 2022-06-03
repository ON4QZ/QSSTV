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
#include "configdialog.h"
#include "ui_configdialog.h"
#include "gallerywidget.h"
#include "mainwindow.h"
#include "txwidget.h"
#include "filewatcher.h"


configDialog::configDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::configDialog)
{
  ui->setupUi(this);
  ui->catWidget->attachRigController(rigControllerPtr);
  ui->configTabWidget->setCurrentIndex(0);
}

configDialog::~configDialog()
{
  delete ui;
}

void configDialog::readSettings()
{
  ui->cwWidget->readSettings();
  ui->directoryWidget->readSettings();
  ui->drmProfilesWidget->readSettings();
  ui->ftpWidget->readSettings();
  ui->guiWidget->readSettings();
  ui->hybridWidget->readSettings();
  ui->operatorWidget->readSettings();
  ui->repeaterWidget->readSettings();
  ui->catWidget->readSettings();
  ui->soundWidget->readSettings();
  ui->waterfallWidget->readSettings();
  ui->freqSelectWidget->readSettings();
}

void configDialog::writeSettings()
{
  ui->operatorWidget->writeSettings();
  ui->directoryWidget->writeSettings();
  ui->soundWidget->writeSettings();
  ui->guiWidget->writeSettings();
  ui->catWidget->writeSettings();
  ui->ftpWidget->writeSettings();
  ui->repeaterWidget->writeSettings();
  ui->hybridWidget->writeSettings();
  ui->drmProfilesWidget->writeSettings();
  ui->cwWidget->writeSettings();
  ui->waterfallWidget->writeSettings();
  soundNeedsRestart=ui->soundWidget->hasChanged();
  guiNeedsRestart=ui->guiWidget->hasChanged();
  ui->freqSelectWidget->writeSettings();
  if(ui->guiWidget->backGroundColorChanged)
    {
      emit bgColorChanged();
      ui->guiWidget->backGroundColorChanged=false;
    }

}

/**
    Opens the configuration dialog
*/

int configDialog::exec()
{
  if(QDialog::exec())
  {
    writeSettings();
    if(ui->directoryWidget->hasChanged() || ui->guiWidget->hasChanged())
      {
        galleryWidgetPtr->changedMatrix();
      }
    if(ui->drmProfilesWidget->hasChanged())
      {
        txWidgetPtr->reloadProfiles();
      }
    if(ui->directoryWidget->hasChanged())
      {
        fileWatcherPtr->init();
      }
    mainWindowPtr->setSSTVDRMPushButton(transmissionModeIndex==TRXDRM);

    return QDialog::Accepted;
  }
  else
  {
    return QDialog::Rejected;
  }
}


