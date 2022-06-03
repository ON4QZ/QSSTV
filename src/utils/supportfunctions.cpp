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

#include "supportfunctions.h"
#include "appglobal.h"
#include <QDateTime>
#include <QDebug>
#include <stdarg.h>
#include "dirdialog.h"


QString lastPath("");

bool getValue(int &val, QLineEdit* input)
{
	bool ok;
	QString s;
	s=input->text();
	val=s.toInt(&ok,0); // allow ayutomatic conversion from hex to decimal in the classic C++ way : 0x is hex other are decimal
	return ok;
}

bool getValue(double &val, QLineEdit* input)
{
	bool ok;
	QString s;
	s=input->text();
	val=s.toDouble(&ok);
	return ok;
}

bool getValue(int &val, QString input)
{
	bool ok;
	val=input.toInt(&ok);
	return ok;
}
bool getValue(double &val, QString input)
{
	bool ok;
	val=input.toDouble(&ok);
	return ok;
}

void getValue(bool &val, QCheckBox *input)
{
	val=input->isChecked();
}

void getValue(int &val, QSpinBox *input)
{
	val=input->value();
}

void getValue(uint &val, QSpinBox *input)
{
  val=input->value();
}

void getValue(double &val, QDoubleSpinBox *input)
{
  val=input->value();
}

void getValue(QString &s, QLineEdit *input)
{
	s=input->text();
}

void getValue(QString &s, QPlainTextEdit *input)
{
  s=input->toPlainText();
}



void getValue(int &s, QComboBox *input)
{
	s=input->currentText().toInt();
}

void getIndex(int &s, QComboBox *input)
{
  s=input->currentIndex();
}

void getValue(QString &s, QComboBox *input)
{
	s=input->currentText();
}

void getValue(bool &val, QPushButton *input)
{
  val=input->isChecked();
}


void getValue(int &s, QButtonGroup *input)
{
	s=input->checkedId();
}

void getValue(bool &s, QRadioButton *input)
{
	s=input->isChecked();
}

void getValue(int &val, QSlider *input)
{
  val=input->value();
}

void getValue(uint &val, QSlider *input)
{
  val=input->value();
}

void setValue(int val, QLineEdit* output)
{
	output->setText(QString::number(val));
}
 
void setValue(double val, QLineEdit* output)
{
	output->setText(QString::number(val));
}
/**
	\brief sets double number in a QlineEdit
	\param val  the value to set
	\param output pointer to QLineEdit
	\param prec the required precision
*/
 
void setValue(double val, QLineEdit* output,int prec)
{
	output->setText(QString::number(val,'g',prec));
}

void setValue(bool val, QCheckBox *input)
{
	input->setChecked(val);
}

void setValue(int val, QSpinBox *input)
{
	input->setValue(val);
}

void setValue(uint val, QSpinBox *input)
{
  input->setValue(val);
}

void setValue(double val, QDoubleSpinBox *input)
{
  input->setValue(val);
}

void setValue(QString s, QLineEdit *input)
{
	input->setText(s);
}

void setValue(QString s, QPlainTextEdit *input)
{
  input->setPlainText(s);
}

void setValue(int s, QComboBox *input)
{
	int i;
	for(i=0;i<input->count();i++)
		{
			if(input->itemText(i).toInt()==s)
				{
					input->setCurrentIndex(i);
					return;
				}
		}
	input->setCurrentIndex(0);
}

void setIndex(int s, QComboBox *input)
{
  input->setCurrentIndex(s);
}

void setValue(QString s, QComboBox *input)
{
	int i;
	for(i=0;i<input->count();i++)
		{
			if(input->itemText(i)==s)
				{
					input->setCurrentIndex(i);
					return;
				}
		}
	input->setCurrentIndex(0);
}

void setValue(bool s, QPushButton *input)
{
  input->setChecked(s);
}

void setValue(int s, QButtonGroup *input)
{
	input->button(s)->setChecked(true);
}

void setValue(bool s, QRadioButton *input)
{
	input->setChecked(s);
}

void setValue(int val, QSlider *input)
{
  input->setValue(val);
}


bool browseGetFile(QLineEdit *le,QString deflt, const QString &filter)
{
    dirDialog d((QWidget *)le,"Browse");
    QString s=d.openFileName(deflt,filter);
  if (s.isNull()) return false;
	if (s.isEmpty()) return false;
	le->setText(s);
	return true;
}

bool browseSaveFile(QLineEdit *le,QString deflt,const QString &filter)
{
    dirDialog d((QWidget *)le,"Browse");
	QString s=d.saveFileName(deflt,filter,"");
  if (s.isNull()) return false;
	if (s.isEmpty()) return false;
	le->setText(s);
	return true;
}

bool browseDir(QLineEdit *le,QString deflt)
{
    dirDialog d((QWidget *)le,"Browse");
    QString s=d.openDirName(deflt);
  if (s.isNull()) return false;
	if (s.isEmpty()) return false;
	le->setText(s);
	return true;
}




void deleteFiles(QString dirPath,QString extension)
{
  int i;
  QDir dir(dirPath);
  QStringList filters;
  QFile fi;
  filters << extension;
  dir.setNameFilters(filters);
  QFileInfoList entries = dir.entryInfoList(filters,QDir::Files|QDir::NoSymLinks);
  for(i=0;i<entries.count();i++)
    {
      fi.setFileName(entries.at(i).absoluteFilePath());
      fi.remove();
    }
}

bool trash(QString filename,bool forceDelete)
{
  QString tmp;
  QFile orgFile(filename);
  QFileInfo modifiedFileInfo(filename);
  QFileInfo info(filename);
  QFile infoFile;
  QFile modifiedFile;
  QDir trDir;
  QDir infoDir;
  QDir filesDir;
  QString infoTxt;
  trDir.setPath(getenv("XDG_DATA_HOME"));
  if (trDir.path().isEmpty())  trDir.setPath(QDir::homePath()+"/.local/share/Trash");
  infoDir.setPath(trDir.path()+"/info");
  filesDir.setPath(trDir.path()+"/files");
  infoFile.setFileName(infoDir.path()+"/"+info.fileName()+".trashinfo");
  modifiedFile.setFileName(filesDir.path()+"/"+modifiedFileInfo.fileName());
  int counter=0;
  do
    {
      if(!modifiedFile.exists()) break;
      counter++;
      tmp=QString("%1/%2_%3.%4").arg(filesDir.path()).arg(modifiedFileInfo.completeBaseName()).arg(QString::number(counter)).arg(modifiedFileInfo.suffix());
      modifiedFile.setFileName(tmp);
      tmp=QString("%1/%2_%3.%4").arg(infoDir.path()).arg(modifiedFileInfo.completeBaseName()).arg(QString::number(counter)).arg(modifiedFileInfo.suffix());
      infoFile.setFileName(tmp+".trashinfo");
    }
  while(1);

  infoTxt=QString("[Trash Info]\nPath=%1\nDeletionDate=%2")
      .arg(filename).arg(QDateTime::currentDateTime().toString(Qt::ISODate));

  if((!trDir.exists()) || (!infoDir.exists()) || (!filesDir.exists()))
    {
      errorOut() << "Trash folder or one of its components does not exist";
      if(forceDelete) orgFile.remove();
      return false;
    }

  if(!infoFile.open(QIODevice::WriteOnly))
  {
   errorOut() << QString("Trash folder: can't open %1 for writing").arg(infoFile.fileName());
    if(forceDelete) orgFile.remove();
    return false;
  }
  infoFile.write(infoTxt.toLatin1().data());
  infoFile.close();
  QFile trashFile(info.absoluteFilePath());
  QString target;
  target=QString("%1").arg(modifiedFile.fileName());
  if(!trashFile.rename(filename,target))
  {
    errorOut() << QString("Trash folder: can't rename %1 to %2").arg(filename).arg(target);
    if(forceDelete) orgFile.remove();
    return false;
  }
  return true;
}


timingAnalyser::timingAnalyser()
{

}

timingAnalyser::~timingAnalyser()
{
}
void timingAnalyser::start()
{
  tm.start();
}

unsigned long timingAnalyser::result()
{
  return tm.elapsed();
}






