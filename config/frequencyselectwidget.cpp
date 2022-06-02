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

#include "frequencyselectwidget.h"
#include "ui_frequencyselectwidget.h"

QStringList freqList;
QStringList modeList;
QStringList sbModeList;
QStringList passBandList;
QString additionalCommand;
bool additionalCommandHex;

frequencySelectWidget::frequencySelectWidget(QWidget *parent) :baseConfig(parent),
  ui(new Ui::frequencySelectWidget)
{
  ui->setupUi(this);
  QStringList sl;
  sl<<"Frequencyy"<<"Mode"<<"Modulation"<<"Passband";
  ui->tableWidget->setAlternatingRowColors (false);
  ui->tableWidget->setColumnCount(4);
  ui->tableWidget->setHorizontalHeaderLabels(sl);
  connect(ui->tableWidget,SIGNAL(itemChanged(QTableWidgetItem *)),this,SLOT(slotItemChanged()));
  connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(slotCellClicked(int,int)));
  connect(ui->addFreqPushButton,SIGNAL(clicked()),this,SLOT(slotFreqAdd()));
  connect(ui->deleteFreqPushButton,SIGNAL(clicked()),this,SLOT(slotFreqDelete()));
  connect(ui->moveUpFreqPushButton,SIGNAL(clicked()),this,SLOT(slotFreqUp()));
  connect(ui->moveDownFreqPushButton,SIGNAL(clicked()),this,SLOT(slotFreqDown()));
  lastRowSelected=-1;
}

frequencySelectWidget::~frequencySelectWidget()
{
  writeSettings();
  delete ui;

}

void frequencySelectWidget::readSettings()
{
  int i;
  QSettings qSettings;
  qSettings.beginGroup("FREQSELECT");
  freqList=qSettings.value("frequencyList",QStringList()).toStringList();
  modeList=qSettings.value("modeList",QStringList()).toStringList();
  sbModeList=qSettings.value("sbModeList",QStringList()).toStringList();
  passBandList=qSettings.value("passBandList",QStringList()).toStringList();
  additionalCommand=qSettings.value("additionalCommand",QString()).toString();
  additionalCommandHex=qSettings.value("additionalCommandHex",false).toBool();
  if(passBandList.count()!=freqList.count())
    {
      passBandList.clear();
      for(i=0;i<freqList.count();i++)
        {
          passBandList<<"Normal";
        }
    }

  if(modeList.count()!=freqList.count()  || sbModeList.count()!=freqList.count())
    {
      // invalid config
      freqList.clear();
      modeList.clear();
      sbModeList.clear();
      passBandList.clear();
    }
  setParams();
  qSettings.endGroup();
}

void frequencySelectWidget::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("FREQSELECT");
  qSettings.setValue("frequencyList",freqList);
  qSettings.setValue("modeList",modeList);
  qSettings.setValue("sbModeList",sbModeList);
  qSettings.setValue("passBandList",passBandList);
  qSettings.setValue("additionalCommand",additionalCommand);
  qSettings.setValue("additionalCommandHex",additionalCommandHex);
  qSettings.endGroup();
}

void frequencySelectWidget::constructTable()
{
  int i;
  while(ui->tableWidget->rowCount()>0)
    {
      ui->tableWidget->removeRow(0);
    }
  ui->tableWidget->setRowCount(freqList.count());

  for(i=0;i<freqList.count();)
    {
      if(!freqList.at(i).isEmpty())
        {
          createEntry(i++);
        }
      else
        {
          freqList.takeAt(i);
          modeList.takeAt(i);
          sbModeList.takeAt(i);
          passBandList.takeAt(i);
          ui->tableWidget->setRowCount(freqList.count());
        }
    }
}

void frequencySelectWidget:: getParams()
{
  int i;
  bool ok;
  freqList.clear();
  modeList.clear();
  sbModeList.clear();
  passBandList.clear();

  for(i=0;i<ui->tableWidget->rowCount();i++)
    {
      ui->tableWidget->item(i,0)->text().toDouble(&ok);
      {
        if (!ok)
          {
            blockSignals(true);
            ui->tableWidget->item(i,0)->setText("");
            blockSignals(false);
          }
      }
      freqList.append(ui->tableWidget->item(i,0)->text());
      modeList.append(((QComboBox *)ui->tableWidget->cellWidget(i,1))->currentText());
      sbModeList.append(((QComboBox *)ui->tableWidget->cellWidget(i,2))->currentText());
      passBandList.append(((QComboBox *)ui->tableWidget->cellWidget(i,3))->currentText());
    }
  getValue(additionalCommand,ui->additionalCommandLineEdit);
  getValue(additionalCommandHex,ui->additionalCommandHexCheckBox);
}

void frequencySelectWidget::setParams()
{
  constructTable();
  setValue(additionalCommand,ui->additionalCommandLineEdit);
  setValue(additionalCommandHex,ui->additionalCommandHexCheckBox);
}







void frequencySelectWidget::slotFreqAdd()
{
  freqList.append("");
  modeList.append("SSTV");
  sbModeList.append("LSB");
  passBandList.append("Normal");
  createEntry(freqList.count()-1);
}

void frequencySelectWidget::slotFreqDelete()
{
  int curRow=lastRowSelected;
  freqList.takeAt(curRow);
  modeList.takeAt(curRow);
  sbModeList.takeAt(curRow);
  constructTable();
  if((curRow<freqList.count()) && (curRow!=0))
    {
      ui->tableWidget->setCurrentCell(curRow-1,0);
      ui->tableWidget->item(curRow-1, 0)->setSelected(true);
      setLastRowSelected();
    }
}
void frequencySelectWidget::slotFreqUp()
{
  int curRow=lastRowSelected;
  QString  f,m,sb;
  if(curRow>0)
    {
      f=freqList.at(curRow-1);
      m=modeList.at(curRow-1);
      sb=sbModeList.at(curRow-1);
      freqList[curRow-1]=freqList.at(curRow);
      modeList[curRow-1]=modeList.at(curRow);
      sbModeList[curRow-1]=sbModeList.at(curRow);
      freqList[curRow]=f;
      modeList[curRow]=m;
      sbModeList[curRow]=sb;
      constructTable();
      ui->tableWidget->setCurrentCell(curRow-1,0);
      ui->tableWidget->item(curRow-1, 0)->setSelected(true);
      setLastRowSelected();

    }
}

void frequencySelectWidget::slotFreqDown()
{
  int curRow=lastRowSelected;
  QString  f,m,sb;
  if(curRow<(ui->tableWidget->rowCount()-1) && curRow>=0)
    {
      f=freqList.at(curRow+1);
      m=modeList.at(curRow+1);
      sb=sbModeList.at(curRow+1);
      freqList[curRow+1]=freqList.at(curRow);
      modeList[curRow+1]=modeList.at(curRow);
      sbModeList[curRow+1]=sbModeList.at(curRow);
      freqList[curRow]=f;
      modeList[curRow]=m;
      sbModeList[curRow]=sb;
      constructTable();
      ui->tableWidget->setCurrentCell(curRow+1,0);
      ui->tableWidget->item(curRow+1, 0)->setSelected(true);
      setLastRowSelected();

    }
}

void frequencySelectWidget::createEntry(int row)
{
  QComboBox *cb, *sb, *pb;
  QTableWidgetItem *ct;
  if(row>(ui->tableWidget->rowCount()-1))
    {
      ui->tableWidget->setRowCount(row+1);
    }
  ui->tableWidget-> blockSignals(true);
  ct=new QTableWidgetItem();
  ct->setText(freqList.at(row));
  ui->tableWidget->setItem(row,0,ct);

  cb=new QComboBox(this);
  cb->addItem("SSTV");
  cb->addItem("DRM");
//  cb->setCurrentText(modeList.at(row));
  setValue(modeList.at(row),cb);

  ui->tableWidget->setCellWidget(row,1,cb);

  sb=new QComboBox(this);
  sb->addItem("LSB");
  sb->addItem("USB");
  sb->addItem("FM");
  sb->addItem("AM");
  sb->addItem("PKTLSB");
  sb->addItem("PKTUSB");
  sb->addItem("PKTFM");



//  sb->setCurrentText(sbModeList.at(row));
  setValue(sbModeList.at(row),sb);
  ui->tableWidget->setCellWidget(row,2,sb);

  pb=new QComboBox(this);
  pb->addItem("Normal");
  pb->addItem("Wide");
  pb->addItem("Narrow");
//  sb->setCurrentText(sbModeList.at(row));
  setValue(passBandList.at(row),pb);
  ui->tableWidget->setCellWidget(row,3,pb);
  connect(cb,SIGNAL(currentIndexChanged(int)),SLOT(slotItemChanged()));
  connect(sb,SIGNAL(currentIndexChanged(int)),SLOT(slotItemChanged()));
   ui->tableWidget->blockSignals(false);
}

void frequencySelectWidget::slotItemChanged()
{
  getParams();
}

void frequencySelectWidget::setLastRowSelected()
{
  lastRowSelected=ui->tableWidget->currentRow();
  if(lastRowSelected>=ui->tableWidget->rowCount())
    {
      lastRowSelected=-1;
    }
}
void frequencySelectWidget::slotCellClicked(int r,int)
{
  lastRowSelected=r;
}
