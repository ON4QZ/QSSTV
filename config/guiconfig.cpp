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

#include "guiconfig.h"
#include "ui_guiconfig.h"
#include <QImageWriter>
#include <QColorDialog>


int galleryRows;
int galleryColumns;
bool imageStretch;
QColor backGroundColor;
QColor imageBackGroundColor;
bool slowCPU;
bool lowRes;
bool confirmDeletion;
bool confirmClose;

guiConfig::guiConfig(QWidget *parent) : baseConfig(parent),  ui(new Ui::guiConfig)
{
  ui->setupUi(this);
  backGroundColorChanged=false;
  connect(ui->backgroundColorPushButton,SIGNAL(clicked()),SLOT(slotBGColorSelect()));
  connect(ui->imageBackgroundColorPushButton,SIGNAL(clicked()),SLOT(slotIBGColorSelect()));
}

guiConfig::~guiConfig()
{
  delete ui;
}

void guiConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("GUI");
  galleryRows=qSettings.value("galleryRows",4).toInt();
  galleryColumns=qSettings.value("galleryColumns",4).toInt();
  imageStretch=qSettings.value("imageStretch",true).toBool();
  backGroundColor=qSettings.value("backGroundColor",QColor(0,86,230)).value<QColor>();
  imageBackGroundColor=qSettings.value("imageBackGroundColor",QColor(128,128,128)).value<QColor>();
  slowCPU=qSettings.value("slowCPU",false).toBool();
  lowRes=qSettings.value("lowRes",false).toBool();
  confirmDeletion=qSettings.value("confirmDeletion",true).toBool();
  confirmClose=qSettings.value("confirmClose",true).toBool();
  qSettings.endGroup();
  setParams();
}

void guiConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("GUI");
  qSettings.setValue("galleryRows",galleryRows);
  qSettings.setValue("galleryColumns",galleryColumns);
  qSettings.setValue("imageStretch",imageStretch);
  qSettings.setValue("backGroundColor",backGroundColor);
  qSettings.setValue("imageBackGroundColor",imageBackGroundColor);
  qSettings.setValue("slowCPU",slowCPU);
  qSettings.setValue("lowRes",lowRes);
  qSettings.setValue("confirmDeletion",confirmDeletion);
  qSettings.setValue("confirmClose",confirmClose);


  qSettings.endGroup();
}

void guiConfig::getParams()
{
  int galleryRowsCopy=galleryRows;
  int galleryColumnsCopy=galleryColumns;
  getValue(galleryRows,ui->rowsSpinBox);
  getValue(galleryColumns, ui->columnsSpinBox);
  changed=false;
  if( galleryRowsCopy!=galleryRows || galleryColumnsCopy!=galleryColumns)
    changed=true;
  getValue(imageStretch,ui->stretchCheckBox);
  getValue(slowCPU,ui->slowCPUCheckBox);
  getValue(lowRes,ui->lowResCheckBox);

  getValue(confirmDeletion,ui->confirmDeletionCheckBox);
  getValue(confirmClose,ui->confirmCloseCheckBox);

}

void guiConfig::setParams()
{
  setValue(galleryRows,ui->rowsSpinBox);
  setValue(galleryColumns, ui->columnsSpinBox);
  setValue(imageStretch,ui->stretchCheckBox);
  setColorLabel(backGroundColor,false);
  setColorLabel(imageBackGroundColor,true);
  setValue(slowCPU,ui->slowCPUCheckBox);
  setValue(lowRes,ui->lowResCheckBox);
  setValue(confirmDeletion,ui->confirmDeletionCheckBox);
  setValue(confirmClose,ui->confirmCloseCheckBox);
}


void guiConfig::slotBGColorSelect()
{
  QColor c;
  c=QColorDialog::getColor(backGroundColor,this,"",QColorDialog::ShowAlphaChannel);
  setColorLabel(c,false);
}

void guiConfig::slotIBGColorSelect()
{
  QColor c;
  c=QColorDialog::getColor(imageBackGroundColor,this,"",QColorDialog::ShowAlphaChannel);
  setColorLabel(c,true);
}


void guiConfig::setColorLabel(QColor c,bool image)
{
if (c.isValid())
  {
    if(!image)
      {
        backGroundColorChanged=true;
        QPalette palette = ui->backGroundColorLabel->palette();
        palette.setColor(ui->backGroundColorLabel->backgroundRole(), c);
        ui->backGroundColorLabel->setPalette(palette);
        backGroundColor=c;
      }
    else
      {
        QPalette palette = ui->imageBackGroundColorLabel->palette();
        palette.setColor(ui->imageBackGroundColorLabel->backgroundRole(), c);
        ui->imageBackGroundColorLabel->setPalette(palette);
        imageBackGroundColor=c;
      }
   }
}
