/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   https://www.qsl.net/o/on4qz                                           *
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
#include "rigcontrol.h"
#include "appglobal.h"

#include "mainwindow.h"
#include "txwidget.h"
#include "configparams.h"
#include "dispatcher.h"


#include <QSplashScreen>
#include <QMessageBox>
#include <QApplication>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>


#define MAXCONFLEN 128
#ifndef HAMLIB_FILPATHLEN
#define HAMLIB_FILPATHLEN FILPATHLEN
#endif


QList<const rig_caps *> capsList;
bool radiolistLoaded=false;




int collect(const rig_caps *caps,rig_ptr_t)
{
  capsList.append(caps);
  return 1;
}

rigControl::rigControl(int radioIndex)
{
  rigControlEnabled=false;
  catParams.configLabel=QString("radio%1").arg(radioIndex);
  rig_set_debug(RIG_DEBUG_NONE);
  getRadioList();
  serialP=0;
  lastFrequency=0.0;
  xmlModes<<"USB"<<"LSB"<<"FM"<<"AM";
}

rigControl::~rigControl()
{
  rig_close(my_rig); /* close port */
  rig_cleanup(my_rig); /* if you care about memory */
}

bool rigControl::init()
{
  int retcode;
  if(!catParams.enableCAT) return false;

  catParams.radioModelNumber=getModelNumber(getRadioModelIndex());
  my_rig = rig_init(catParams.radioModelNumber);
  if(!my_rig)
    {
      addToLog(QString("Error in connection using radio model %1").arg(catParams.radioModel),LOGALL);
      initError=QString("Error in connection using radio model %1").arg(catParams.radioModel);
      return false;
    }

  if(QString(my_rig->caps->mfg_name)=="Icom")
    {
      if(!catParams.civAddress.isEmpty())
        {
          rig_set_conf(my_rig, rig_token_lookup(my_rig, "civaddr"), catParams.civAddress.toLatin1());
        }
    }
  if(!catParams.serialPort.isEmpty())
    {
      strncpy(my_rig->state.rigport.pathname,(const char *)catParams.serialPort.toLatin1().data(),HAMLIB_FILPATHLEN-1);
    }
  my_rig->state.rigport.parm.serial.rate = catParams.baudrate;
  my_rig->state.rigport.parm.serial.data_bits=catParams.databits;
  my_rig->state.rigport.parm.serial.stop_bits=catParams.stopbits;
  if(catParams.parity=="Even") my_rig->state.rigport.parm.serial.parity= RIG_PARITY_EVEN;
  else if (catParams.parity=="Odd") my_rig->state.rigport.parm.serial.parity = RIG_PARITY_ODD;
  else  my_rig->state.rigport.parm.serial.parity = RIG_PARITY_NONE;
  if(catParams.handshake=="XOn/Xoff") my_rig->state.rigport.parm.serial.handshake = RIG_HANDSHAKE_XONXOFF;
  if(catParams.handshake=="Hardware") my_rig->state.rigport.parm.serial.handshake = RIG_HANDSHAKE_HARDWARE;
  else my_rig->state.rigport.parm.serial.handshake = RIG_HANDSHAKE_NONE;
  my_rig->state.pttport.type.ptt = catParams.pttType;

  addToLog(QString("rigcontrol:init rigport.pathname: %1").arg(my_rig->state.rigport.pathname),LOGRIGCTRL);
  retcode = rig_open(my_rig);

  if (retcode != RIG_OK )
    {
      addToLog(QString("CAT Error: %1").arg(QString(rigerror(retcode))),LOGALL);
      initError=QString("CAT Error: %1").arg(QString(rigerror(retcode)));
      return false;
    }
  addToLog("rigcontroller successfully opened",LOGRIGCTRL);
  rigControlEnabled=true;



  // int verbose=0;
  // rig_set_debug(verbose<2 ? RIG_DEBUG_NONE: (rig_debug_level_e)verbose);
  // rig_debug(RIG_DEBUG_VERBOSE, "rigctl, %s\n", hamlib_version);
  // test if we can contact the transceiver

  canSetFreq=(my_rig->caps->set_freq != NULL);
  canGetFreq=(my_rig->caps->get_freq != NULL);
  canSetMode=(my_rig->caps->set_mode != NULL);
  canGetMode=(my_rig->caps->get_mode != NULL);
  canSetPTT=(my_rig->caps->set_ptt != NULL) ||
          (my_rig->state.pttport.type.ptt == RIG_PTT_SERIAL_DTR) ||
          (my_rig->state.pttport.type.ptt == RIG_PTT_SERIAL_RTS);
  canGetPTT=(my_rig->caps->get_ptt != NULL);
  double fr;
  getFrequency(fr);
  return true;
}


bool rigControl::getFrequency(double &frequency)
{
  int retcode;
  if(catParams.enableXMLRPC)
    {
      frequency=xmlIntfPtr->getFrequency();
      return true;
    }

  if(!rigControlEnabled || !canGetFreq) return false;
  retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &frequency);
  for(int i=0;i<RIGCMDTRIES;i++)
    {
      qDebug() << "getFreq";
      retcode = rig_get_freq(my_rig, RIG_VFO_CURR, &frequency);
      qDebug() << "got Freq";
      if (retcode==RIG_OK)
        {
          return true;
        }
    }
  //  errorMessage(retcode,"getFrequency");
  canGetFreq=false; // too many errors;
  frequency=lastFrequency;
  return false;

}

bool rigControl::setFrequency(double frequency)
{
  int retcode=-1;
  if(catParams.enableXMLRPC)
    {
      xmlIntfPtr->setFrequency(frequency);
      return true;
    }
  if(!rigControlEnabled || !canSetFreq) return false;
  //        retcode = rig_set_vfo(my_rig, RIG_VFO_CURR);
  //        if (retcode != RIG_OK ) {errorMessage(retcode,"setVFO"); return false; }

  for(int i=0;i<RIGCMDTRIES;i++)
    {
      retcode = rig_set_freq(my_rig, RIG_VFO_CURR, frequency);
      if (retcode==RIG_OK)
        {
          return true;
        }
    }
  errorMessage(retcode,"setFrequency");
  canSetFreq=false; // too many errors;
  return false;
}

void rigControl::disable()
{
  if(rigControlEnabled)
    {
      rig_close(my_rig); /* close port */
      rig_cleanup(my_rig);
      rigControlEnabled=false;
    }
}

bool rigControl::getMode(QString &mode)
{
  if(catParams.enableXMLRPC)
    {
      mode =xmlIntfPtr->getMode();
      return true;
    }

  rmode_t rmode;
  pbwidth_t width;
  int retcode;
  if(!rigControlEnabled || !canGetMode) return false;

  for(int i=0;i<RIGCMDTRIES;i++)
    {
      retcode = rig_get_mode(my_rig, RIG_VFO_CURR, &rmode, &width);
      if (retcode==RIG_OK)
        {
          mode=QString(rig_strrmode(rmode));
          return true;
        }
    }
  canGetMode=false; // too many errors
  errorMessage(retcode,"getMode");
  return false;


}

bool rigControl::setMode(QString mode,QString passBand)
{
  QString orgMode;
  int pb;
  int i;
  int pos=-1;
  if(catParams.enableXMLRPC)
    {
      orgMode=xmlIntfPtr->getMode();
      for(i=0;i<xmlModes.count();i++)
        {
          pos=orgMode.indexOf(xmlModes.at(i),Qt::CaseInsensitive);
          if(pos>=0)
            {
              orgMode.replace(xmlModes.at(i),mode);
              break;
            }
        }
      if(pos>=0) xmlIntfPtr->setMode(orgMode);
      return true;
    }

  rmode_t rmode=rig_parse_mode(mode.toLatin1().data());
  if(passBand=="Narrow")
    {
      pb=rig_passband_narrow(my_rig,rmode);
    }
  else if(passBand=="Wide")
    {
      pb=rig_passband_wide(my_rig,rmode);
    }
  else
    {
      pb=rig_passband_normal(my_rig,rmode);
    }
  int retcode;
  if(!rigControlEnabled || !canSetMode) return false;

  for(int i=0;i<RIGCMDTRIES;i++)
    {
      retcode = rig_set_mode(my_rig, RIG_VFO_CURR, rmode, pb);
      if (retcode==RIG_OK)
        {
          mode=QString(rig_strrmode(rmode));
          return true;
        }
    }
  canSetMode=false; // too many errors
  errorMessage(retcode,"setMode");
  return false;
}


bool rigControl::setPTT(bool on)
{
  int retcode;
  ptt_t ptt;
  /* Hamlib will fall back to RIG_PTT_ON if RIG_PTT_ON_DATA is not available in current hamlib configuration */
  if(on) ptt=RIG_PTT_ON_DATA; else ptt=RIG_PTT_OFF;
  if(!rigControlEnabled || !canSetPTT) return false;

  for(int i=0;i<RIGCMDTRIES;i++)
    {
      retcode = rig_set_ptt (my_rig, RIG_VFO_CURR,ptt);
      if (retcode==RIG_OK)
        {
          return true;
        }
    }
  errorMessage(retcode,"setPTT");
  return false;
}



void  rigControl::errorMessage(int errorCode,QString command)
{
  displayMBoxEvent *stmb;
  stmb= new displayMBoxEvent("Cat interface",QString("Error in connection: %1\n%2").arg(QString(rigerror(errorCode))).arg(command));
  QApplication::postEvent( dispatcherPtr, stmb );

  //  QMessageBox::information(0,"Cat interface",QString("Error in connection: %1\n%2").arg(QString(rigerror(errorCode))).arg(command));
}

void rigControl::getRadioList()
{
  if(!radiolistLoaded)
    {
      capsList.clear();
      rig_load_all_backends();
      rig_list_foreach(collect,0);
      std::sort(capsList.begin(),capsList.end(),model_Sort);
      radiolistLoaded=true;
    }
}

bool rigControl::getRadioList(QComboBox *cb)
{
  int i;
  if(capsList.count()==0) return false;
  QStringList sl;
  for (i=0;i<capsList.count();i++)
    {
      QString t;
      t= QString::number(capsList.at(i)->rig_model);
      t=t.rightJustified(5,' ')+" ";
      t+= capsList.at(i)->mfg_name;
      t+=",";
      t+=capsList.at(i)->model_name;
      sl << t;
    }
  cb->addItems(sl);
  return true;
}

int rigControl::getModelNumber(int idx)
{
  if(idx<0) return 0;
  return capsList.at(idx)->rig_model;
}

int rigControl::getRadioModelIndex()
{
  int i;
  QString t=catParams.radioModel;
  t=t.remove(0,5);
  t=t.simplified();
  QStringList sl=t.split(",");
  if(sl.count()==1) sl.append("");
  for(i=0;i<capsList.count();i++)
    {
      if((capsList.at(i)->mfg_name==sl.at(0)) && (capsList.at(i)->model_name==sl.at(1)))
        {
          return i;
        }
    }
  return -1;
}

bool model_Sort(const rig_caps *caps1,const rig_caps *caps2)
{
  if(caps1->mfg_name==caps2->mfg_name)
    {
      if (QString::compare(caps1->model_name,caps2->model_name)<0) return true;
      return false;
    }
  if (QString::compare(caps1->mfg_name,caps2->mfg_name)<0) return true;
  return false;
}

void rigControl::activatePTT(bool b)
{
  int modemlines;
  if(catParams.enableSerialPTT)
    {
      if (catParams.pttSerialPort.isEmpty()) return;
      if(serialP==0)
        {
          serialP=::open(catParams.pttSerialPort.toLatin1().data(),O_RDWR|O_NONBLOCK);
          if (serialP<=0)
            {
              QMessageBox::warning(txWidgetPtr,"Serial Port Error",
                                   QString("Unable to open serial port %1\ncheck Options->Configuration\n"
                                           "make sure that you have read/write permission\nIf you do not have a serial port,\n"
                                           "then disable -Serial PTT- option in the configuration").arg(catParams.pttSerialPort) ,
                                   QMessageBox::Ok);
              return;
            }
          else
            {
              ioctl(serialP,TIOCMGET,&modemlines);
              if(catParams.activeDTR) modemlines &= ~TIOCM_DTR;
              if(catParams.activeRTS)modemlines &= ~TIOCM_RTS;
              if(catParams.nactiveDTR) modemlines |= ~TIOCM_DTR;
              if(catParams.nactiveRTS)modemlines |= ~TIOCM_RTS;
              ioctl(serialP,TIOCMSET,&modemlines);
            }
        }
      if(serialP>0)
        {
          if(b)
            {
              ioctl(serialP,TIOCMGET,&modemlines);
              if(catParams.activeDTR) modemlines |= TIOCM_DTR;
              if(catParams.activeRTS)modemlines |= TIOCM_RTS;
              if(catParams.nactiveDTR) modemlines &= ~TIOCM_DTR;
              if(catParams.nactiveRTS)modemlines &= ~TIOCM_RTS;
              ioctl(serialP,TIOCMSET,&modemlines);
              //ioctl(serial,TIOCMBIS,&t);
            }
          else
            {
              ioctl(serialP,TIOCMGET,&modemlines);
              if(catParams.activeDTR) modemlines &= ~TIOCM_DTR;
              if(catParams.activeRTS) modemlines &= ~TIOCM_RTS;
              if(catParams.nactiveDTR) modemlines |= ~TIOCM_DTR;
              if(catParams.nactiveRTS)modemlines |= ~TIOCM_RTS;
              ioctl(serialP,TIOCMSET,&modemlines);
            }
        }
    }
  else if(catParams.enableXMLRPC)
    {
      xmlIntfPtr->activatePTT(b);
    }
  else setPTT(b); // does nothing if rigController is disabled
  mainWindowPtr->setPTT(b);
  if(b)
    {
      addToLog("dispatcher: PTT activated",LOGDISPATCH);
    }
  else
    {
      addToLog("dispatcher: PTT deactivated",LOGDISPATCH);
    }
}


int  rigControl::rawCommand(QByteArray ba)
{
  int i;
  bool result;
  char rxBuffer[100];

  QString command="w ";
  QByteArray cmdBa;
  if(!rigControlEnabled) return 0;
  struct rig_state *rs;
  rs = &my_rig->state;
  // check if backend via rigctld
  if(catParams.radioModelNumber==2)
    {
      // we need to send this as a command string
      if(additionalCommandHex)
        {
          for(i=0;i<ba.count();i++)
            {

              command+="\\0x";
              command+=QString::number((unsigned char)ba.at(i),16);
            }
        }
      else
        {
          command+=ba;
        }

      command+="\n";
      cmdBa=command.toLatin1();
      result=write_block(&rs->rigport,cmdBa.constData(), cmdBa.count());

    }
  else
    {
      result=write_block(&rs->rigport,ba.constData(), ba.count());
    }
  read_block(&rs->rigport,rxBuffer,99);
  return result;

}
