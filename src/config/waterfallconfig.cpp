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

#include "waterfallconfig.h"
#include "ui_waterfallconfig.h"
#include <QFont>


QString startPicWF;
QString endPicWF;
QString fixWF;
QString bsrWF;
QString startBinWF;
QString endBinWF;
QString startRepeaterWF;
QString endRepeaterWF;
QString wfFont;
int wfFontSize;
bool wfBold;
QString sampleString;

waterfallConfig::waterfallConfig(QWidget *parent) : baseConfig(parent),  ui(new Ui::waterfallConfig)
{
  ui->setupUi(this);
  connect(ui->fontComboBox,SIGNAL(currentIndexChanged(QString)),SLOT(slotFontChanged()));
  connect(ui->sizeSpinBox,SIGNAL(valueChanged(int)),SLOT(slotFontChanged()));
  connect(ui->boldCheckBox,SIGNAL(clicked(bool)),SLOT(slotFontChanged()));
}

waterfallConfig::~waterfallConfig()
{
  delete ui;
}


void waterfallConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("WATERFALL");
  startPicWF=qSettings.value("startPicWF","START PIC").toString();
  endPicWF=qSettings.value("endPicWF","END PIC").toString();
  fixWF=qSettings.value("fixWF","FIX").toString();
  bsrWF=qSettings.value("bsrWF","BSR").toString();
  startBinWF=qSettings.value("startBinWF","START BIN").toString();
  endBinWF=qSettings.value("endBinWF","END BIN").toString();

  startRepeaterWF=qSettings.value("startRepeaterWF","START RPT").toString();
  endRepeaterWF=qSettings.value("endRepeaterWF","END RPT").toString();

  wfFont=qSettings.value("wfFont","Arial").toString();
  wfFontSize=qSettings.value("wfFontSize",12).toInt();
  wfBold=qSettings.value("wfBold",false).toBool();
  sampleString=qSettings.value("sampleString","Sample Text").toString();
  qSettings.endGroup();
  setParams();
}

void waterfallConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("WATERFALL");
  qSettings.setValue("startPicWF",startPicWF);
  qSettings.setValue("endPicWF",endPicWF);
  qSettings.setValue("fixWF",fixWF);
  qSettings.setValue("bsrWF",bsrWF);
  qSettings.setValue("startBinWF",startBinWF);
  qSettings.setValue("endBinWF",endBinWF);
  qSettings.setValue("startRepeaterWF",startRepeaterWF);
  qSettings.setValue("endRepeaterWF",endRepeaterWF);
  qSettings.setValue("wfFont",wfFont);
  qSettings.setValue("wfFontSize",wfFontSize);
  qSettings.setValue("wfBold",wfBold);
  qSettings.setValue("sampleString",sampleString);

  qSettings.endGroup();
}

void waterfallConfig::getParams()
{
  getValue(startPicWF,ui->startPicTextEdit);
  getValue(endPicWF,ui->endPicTextEdit);
  getValue(fixWF,ui->fixTextEdit);
  getValue(bsrWF,ui->bsrTextEdit);
  getValue(startBinWF,ui->startBinTextEdit);
  getValue(endBinWF,ui->endBinTextEdit);
  getValue(startRepeaterWF,ui->startRepeaterTextEdit);
  getValue(endRepeaterWF,ui->endRepeaterTextEdit);
  getValue(wfFont,ui->fontComboBox);
  getValue(wfFontSize,ui->sizeSpinBox);
  getValue(wfBold,ui->boldCheckBox);
  getValue(sampleString,ui->sampleLineEdit);
}

void waterfallConfig::setParams()
{
  setValue(startPicWF,ui->startPicTextEdit);
  setValue(endPicWF,ui->endPicTextEdit);
  setValue(fixWF,ui->fixTextEdit);
  setValue(bsrWF,ui->bsrTextEdit);
  setValue(startBinWF,ui->startBinTextEdit);
  setValue(endBinWF,ui->endBinTextEdit);
  setValue(startRepeaterWF,ui->startRepeaterTextEdit);
  setValue(endRepeaterWF,ui->endRepeaterTextEdit);
  ui->fontComboBox->blockSignals(true);
  ui->sizeSpinBox->blockSignals(true);
  setValue(sampleString,ui->sampleLineEdit);
  setValue(wfFont,ui->fontComboBox);
  setValue(wfFontSize,ui->sizeSpinBox);
  ui->fontComboBox->blockSignals(false);
  ui->sizeSpinBox->blockSignals(false);
  setValue(wfBold,ui->boldCheckBox);
  slotFontChanged();


}

void waterfallConfig::slotFontChanged()
{
  getParams();
  QFont f(wfFont);
  f.setBold(wfBold);
  f.setPixelSize(wfFontSize);
  ui->sampleLineEdit->setFont(f);
}


