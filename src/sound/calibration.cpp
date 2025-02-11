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
#include "calibration.h"
#include "ui_calibration.h"
#include <QApplication>
#include "appglobal.h"
#include "soundbase.h"
#include "dispatcher.h"
#include <math.h>

#include "QMessageBox"
#include <QPushButton>


#define ITERATIONS 8000

/**
 * \class calibration
 *
 * Check first if ntp is running and it is synchronised. A dialog window will appear and show the progress of the RX and TX clocks.
 * About 10000 blocks of data will be read/written to calculate the exact timing. If the OK button is pressed, the clocks will be saved for later use.
 *
 **/



/**
 * @brief Calibration constructor
 *
 * @param parent parent widget pointer
 */
calibration::calibration(QWidget *parent) : QDialog(parent),  ui(new Ui::calibration)
{
  ui->setupUi(this);
  init();
}



calibration::~calibration()
{
  delete ui;
}



/**
 * @brief start calibration
 *
 * Call this function to start the calibration process. The use the results, check the return status and get the value of the clocks by calling getRXClock() and getTXClock().
 *
 * \sa getRXClock() \sa getTXClock()
 *
 * @return bool true if calibration is successful. Return false if an error occurred or the dialog was canceled
 */
int calibration::exec()
{

  init();
  show();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  if(!start(true)) return QDialog::Rejected;
  if(!start(false)) return QDialog::Rejected;
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  while(!stopped)
  {
    qApp->processEvents();
  }
  if(!canceled) return QDialog::Accepted;
  return QDialog::Rejected;
}


/**
 * @brief initialize
 *
 * This function is called by exec to initialize the calibration and setup the dialog box
 *
  */

void calibration::init()
{
  stopped=false;
  canceled=false;
  // find out if we working with nanoseconds or microseconds
  ui->rxProgress->setMaximum(ITERATIONS-1);
  ui->txProgress->setMaximum(ITERATIONS-1);
  ui->rxProgress->setValue(0);
  ui->txProgress->setValue(0);
  display(BASESAMPLERATE,ui->rxLCD);
  display(BASESAMPLERATE,ui->txLCD);
  connect(this,SIGNAL(finished(int)),SLOT(hasFinished(int)));

}

/**
 * @brief slot for finish
 *
 * This slot is called when the dialog is closed by pressing CANCEL or OK. It will abort the loop executed in start and set the bool canceled to false or true
 * depending on the CANCEL or OK button being pressed.
 **/

void calibration::hasFinished(int result)
{
  stopped=true;
  if(result==QDialog::Rejected) canceled=true;
}


/**
 * @brief initialize
 *
 * @param[in] isRX: If isRX is set then the receive clock will be calibrated, else the transmit clock will be calibrated.
 *
 * Start is called by exec and performs the clock calibration using NTP (Network Time Protocol).
 * It starts counting when the first 100 blocks are read or when the first 100 blocks are written in order to start with a stable condition.
 * @return bool returns true if calibration is successful or false when either the CANCEL button was pressed or a read NTP time error has occurred.
 *
 **/

bool calibration::start(bool isRX)
{
  unsigned int i;
  double clock=0;
  unsigned int frames;
  double elapsedTime;
  double elapsed=0;

  if(!soundIOPtr->calibrate(isRX))
  {
    QMessageBox::critical(this,"Calibration Error","Souncard not active");
    return false;
  }
  for(i=0;(i<ITERATIONS)&&(!stopped);)
  {
    qApp->processEvents();
    if(soundIOPtr->calibrationCount(frames,elapsedTime))
    {
      i++;
      logFilePtr->addToAux(QString("%1\t%2\t%3").arg(frames).arg(elapsedTime).arg(elapsedTime-elapsed));
      elapsed=elapsedTime;
      if(i%2==0)
        {
        clock=((double)frames*CALIBRATIONSIZE)/(elapsedTime); // debug joma
        if(isRX)
        {
          display((int) round(clock),ui->rxLCD);
          ui->rxProgress->setValue(i);
        }
        else
        {
          display((int) round(clock),ui->txLCD);
          ui->txProgress->setValue(i);
        }
      }
    }
    else
      {
        usleep(10000);
      }
  }
  dispatcherPtr->idleAll();
  if(isRX)
  {
    rxCardClock=clock;
  }
  else
  {
    txCardClock=clock;
  }
  return true;
}


void calibration::display(int value,QLCDNumber *dspl)
{
//  QString tmp=QString::number(value,'g',6);
//  if(tmp.length()==5) tmp+=".0";
  dspl->display(value);
}


