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

#include "logging.h"

#include <QDebug>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include "ui_loggingform.h"


/*! class logFile
  \brief utility class to enable logging facilities

  Create an instance of this class giving the basename of the logfile.
  By default the log is disabled. call setEnabled(true) to enable logging


*/
//logFile logfile;


logFile::logFile() : maskBA(NUMDEBUGLEVELS,false)
{
#ifdef ENABLELOGGING
  lf=new QFile;
#ifdef ENABLEAUX
  auxFile=new QFile;
#endif
#endif
  logCount=0;
  savedLogEntry="";
  savedPosMask=0;
}

/*!
  creates logfile with name=logname, and opens it for writing
*/

#ifdef ENABLELOGGING
bool logFile::open(QString logname)
{

  lf->setFileName(QDir::homePath()+"/"+logname);
#ifdef ENABLEAUX
  auxFile->setFileName(QDir::homePath()+"/aux_"+logname);
#endif
  return reopen();

}
#else
bool logFile::open(QString ) { return true;}
#endif


/*!
  closes the logfile
*/

logFile::~logFile()
{
  close();
}

void logFile::close()
{
#ifdef ENABLELOGGING
  errorOut() << "closing logfile";
  add("End of logfile",LOGALL);
  add("....,",LOGALL);
  delete ts;
  lf->close();
#ifdef ENABLEAUX
  delete auxTs;
  auxFile->close();
#endif
#endif
}

void logFile::reset()
{
  close();
  reopen();
}

bool logFile::reopen()
{
#ifdef ENABLELOGGING
  setEnabled(false);
  QFileInfo finf(*lf);
#ifdef ENABLEAUX
  QFileInfo finfaux(*auxFile);
#endif
  errorOut() << "opening logfile--: " << finf.absoluteFilePath();
  if(!lf->open(QIODevice::WriteOnly))
    {
      errorOut() << "logfile creation failed";
      return false;
    }
#ifdef ENABLEAUX
  errorOut() << "opening logfile: " << finfaux.absoluteFilePath();
  if(!auxFile->open(QIODevice::WriteOnly))
    {
      errorOut() << "auxillary file creation failed";
      lf->close();
      return false;
    }
#endif
  setEnabled(true);
  ts= new QTextStream( lf );
#ifdef ENABLEAUX
  auxTs= new QTextStream( auxFile);
#endif
  savedLogEntry="";
  logCount=0;
  timer.start();
  *ts<< "Time \tElapsed  \t  Level  \t  Count\t          Info\n";
  ts->flush();
#endif
  return true;
}

/*!
  \brief Writes to the logfile

  The output is flushed after every access.Identical messages are only logged once. The count indicates the number of duplicate messages.
*/

#ifdef ENABLELOGGING
void logFile::add(QString message, short unsigned int posMask)
{
  QString tempStr,tempStr_qd;
  if(!(posMask==LOGALL)) // always show messages with DBALL
    {
      if (!maskBA[posMask]) return;
    }
  if (!enabled) return;
  mutex.lock();
  if(logCount==0)
    {
      logCount=1;
      savedLogEntry=message;
      timer.restart();
      tmp=QString("%1 ").arg(timer.elapsed(),5);
      tmp2=QTime::currentTime().toString("HH:mm:ss:zzz ");
      savedPosMask=posMask;
    }
  if ((message==savedLogEntry) &&(deduplicate)) logCount++;
  else
    {
      if(!deduplicate)
        {
          savedLogEntry=message;
          tmp=QString("%1 ").arg(timer.elapsed(),5);
          tmp2 = QTime::currentTime().toString("HH:mm:ss:zzz ");
          savedPosMask=posMask;
        }
      if(savedPosMask==LOGALL)
        {
          tempStr=QString("%1\t%2\tALL     \t%3\t%4\n").arg(tmp2).arg(tmp).arg(logCount).arg(savedLogEntry);
          if(timestamp) tempStr_qd=QString("%1 %2 ALL      %3 %4").arg(tmp2).arg(tmp).arg(logCount).arg(savedLogEntry);
          else  tempStr_qd=QString("ALL      %3 %4").arg(logCount).arg(savedLogEntry);
        }
      else
        {
          tempStr=QString("%1\t%2\t%3\t%4\t%5\n").arg(tmp2).arg(tmp).arg(levelStr[savedPosMask]).arg(logCount).arg(savedLogEntry);
          if(timestamp) tempStr_qd=QString("%1 %2 %3 %4 %5").arg(tmp2).arg(tmp).arg(levelStr[savedPosMask]).arg(logCount).arg(savedLogEntry);
          else tempStr_qd=QString("%3 %4 %5").arg(levelStr[savedPosMask]).arg(logCount).arg(savedLogEntry);
      }
      *ts << tempStr;
      if(outputDebug) qDebug() << tempStr_qd;

//      tmp=QString("%1 ").arg(timer.elapsed(),5);
//      tmp2 = QTime::currentTime().toString("HH:mm:ss:zzz ");
      timer.restart();;
      savedLogEntry=message;
      savedPosMask=posMask;
      logCount=1;
    }
  ts->flush();
  lf->flush();
  mutex.unlock();
}
#else
void logFile::add(QString ,short unsigned int) {}
#endif

void logFile::add(const char *fileName, const char *functionName, int line, QString t, short unsigned int posMask)
{
  QString s;
  QFileInfo finfo(fileName);

  if(debugRef)
    {
      s=QString(finfo.fileName())+":"+QString(functionName)+":"+QString::number(line)+" ";
    }
  s+=t;
  if(s[0]=='\t')
    {
      s.remove(0,1);
    }
  add(s,posMask);
}



#ifdef ENABLEAUX
void logFile::addToAux(QString t)
{
  if (!enabled) return;
  mutex.lock();
  *auxTs << t << "\n";
  auxTs->flush();
  auxFile->flush();
  mutex.unlock();
}
#else
void logFile::addToAux(QString ){}
#endif
/*!
  if enable=true logging wil be performed
  \return previous logging state (true if logging was enabled)
*/

bool logFile::setEnabled(bool enable)
{
  bool t=enabled;
  enabled=enable;
  return t;
}

void logFile::setLogMask(QBitArray logMask)
{
  maskBA=logMask;
}

void logFile::maskSelect(QWidget *wPtr)
{
  int i,j;
  QDialog lf(wPtr);
  QCheckBox *cb;
  //  QTableWidgetItem *item;
  Ui::loggingForm ui;
  ui.setupUi(&lf);
  ui.maskTableWidget->setRowCount((NUMDEBUGLEVELS+1)/2);
  for(i=0;i<ui.maskTableWidget->rowCount();i++)
    {
      for(j=0;(j<2)&(i*2+j<NUMDEBUGLEVELS);j++)
        {
          cb=new QCheckBox(levelStr[i*2+j]);
          cb->setChecked(maskBA[i*2+j]);
          ui.maskTableWidget->setCellWidget(i,j,cb);
        }
    }
  ui.debugCheckBox->setChecked(outputDebug);
  ui.debugRefCheckBox->setChecked(debugRef);
  ui.timestampCheckBox->setChecked(timestamp);
  ui.deduplicateCheckBox->setChecked(deduplicate);

  if(lf.exec()==QDialog::Accepted)
    {
      for(i=0;i<ui.maskTableWidget->rowCount();i++)
        {
          for(j=0;(j<2)&(i*2+j<NUMDEBUGLEVELS);j++)
            {
              cb=(QCheckBox *)ui.maskTableWidget->cellWidget(i,j);
              maskBA[i*2+j]=cb->isChecked();
            }
        }
      outputDebug=ui.debugCheckBox->isChecked();
      deduplicate=ui.deduplicateCheckBox->isChecked();
      //Debug message includes reference
      debugRef=ui.debugRefCheckBox->isChecked();
      timestamp=ui.timestampCheckBox->isChecked();
      writeSettings();
    }
}

void logFile::readSettings()
{
  QBitArray ba;
  QSettings qSettings;
  qSettings.beginGroup ("logging");
  maskBA=qSettings.value("maskBA",QBitArray(NUMDEBUGLEVELS,false)).toBitArray();
  if(maskBA.size()<NUMDEBUGLEVELS)
    {
      maskBA=QBitArray(NUMDEBUGLEVELS,false);
    }
  outputDebug=qSettings.value("outputDebug",false).toBool();
  debugRef=qSettings.value("debugRef",false).toBool();
  deduplicate=qSettings.value("deduplicate",true).toBool();
  timestamp=qSettings.value("timestamp",false).toBool();
  qSettings.endGroup();
}

void logFile::writeSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("logging");
  qSettings.setValue ( "maskBA", maskBA);
  qSettings.setValue("outputDebug",outputDebug);
  qSettings.setValue("debugRef",debugRef);
  qSettings.setValue ( "deduplicate", deduplicate);
  qSettings.setValue ( "timestamp", timestamp);
  qSettings.endGroup();
}


