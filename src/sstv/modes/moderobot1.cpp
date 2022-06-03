/***************************************************************************
 *   Copyright (C) 2005 by Johan Maes   *
 *   on4qz@telenet.be   *
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
 *                                                                         *
 *   In addition, as a special exception, the copyright holders give       *
 *   permission to link the code of this program with any edition of       *
 *   the Qt library by Trolltech AS, Norway (or with modified versions     *
 *   of Qt that use the same license as Qt), and distribute linked         *
 *   combinations including the two.  You must obey the GNU General        *
 *   Public License in all respects for all of the code used other than    *
 *   Qt.  If you modify this file, you may extend this exception to        *
 *   your version of the file, but you are not obligated to do so.  If     *
 *   you do not wish to do so, delete this exception statement from        *
 *   your version.                                                         *
 ***************************************************************************/
#include "moderobot1.h"

// used by Robot36

modeRobot1::modeRobot1(esstvMode m,unsigned int len, bool tx,bool narrowMode):modeBase(m,len,tx,narrowMode)
{
}


modeRobot1::~modeRobot1()
{
}

void modeRobot1::setupParams(double clock)
{
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-blank-syncDuration)/3.;
}

modeBase::embState modeRobot1::rxSetupLine()
{
  if(lineCounter>=activeSSTVParam->numberOfDataLines) return MBENDOFLINE;
  start=lineTimeTableRX[lineCounter];

  switch(subLine)
    {
    case 0:
      debugState=stBP;
      markerFloat=start+bp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 1:
      debugState=stColorLine0;
      calcPixelPositionTable(YLINEODD,false);
      markerFloat+=2*visibleLineLength;
      marker=(unsigned int)round(markerFloat);
      pixelArrayPtr=yArrayPtr;
      return MBPIXELS;
    case 2:
      debugState=stG1;
      avgFreqGap=0;
      avgFreqGapCounter=0;
      markerFloat+=((blank/3)*2);
      marker=(unsigned int)round(markerFloat);
      addToLog(QString("MB1500: marker %1 line:%2").arg(marker).arg(lineCounter),LOGMODES);
      return MB1500;
    case 3:
      debugState=stG1a;
      markerFloat+=(blank/3);
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;

    case 4:
      debugState=stColorLine1;
      calcPixelPositionTable(REDLINE,false);
      pixelArrayPtr=redArrayPtr;
      markerFloat+=visibleLineLength;
      marker=(unsigned int)round(markerFloat);
      return MBPIXELS;
    case 5:
      debugState=stFP;
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 6:
      debugState=stSync;
      markerFloat+=syncDuration;
      marker=(unsigned int)round(markerFloat);
      syncPosition=marker;
      return MBSYNC;
    case 7:
      debugState=stBP;
      markerFloat+=bp;
      marker=(unsigned int)round(markerFloat);
      lineCounter++;
      return MBRXWAIT;
    case 8:
      debugState=stColorLine2;
      calcPixelPositionTable(YLINEEVEN,false);
      pixelArrayPtr=greenArrayPtr;
      markerFloat+=2*visibleLineLength;
      marker=(unsigned int)round(markerFloat);
      return MBPIXELS;
    case 9:
      debugState=stG2;
      avgFreqGap=0;
      avgFreqGapCounter=0;
      markerFloat+=((blank/3)*2);
      marker=(unsigned int)round(markerFloat);
      addToLog(QString("MB2300: marker %1 line: %2").arg(marker).arg(lineCounter),LOGMODES);
      return MB2300;
    case 10:
      debugState=stG2a;
      markerFloat+=(blank/3);
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 11:
      debugState=stColorLine3;
      calcPixelPositionTable(BLUELINE,false);
      pixelArrayPtr=blueArrayPtr;
      markerFloat+=visibleLineLength;
      marker=(unsigned int)round(markerFloat);
      return MBPIXELS;
    case 12:
      debugState=stFP;
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 13:
      debugState=stSync;
      markerFloat+=syncDuration;
      marker=(unsigned int)round(markerFloat);
      syncPosition=marker;
      return MBSYNC;
    default:
      return MBENDOFLINE;
    }
}

void modeRobot1::showLine()
{
  yuvConversion(yArrayPtr);
  addToLog(QString("displayCounter %1 lineCounter:%2").arg(displayLineCounter).arg(lineCounter),LOGMODES);
  yuvConversion(greenArrayPtr);
  addToLog(QString("displayCounter %1 lineCounter:%2").arg(displayLineCounter).arg(lineCounter),LOGMODES);
}


void modeRobot1::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;
  int ofx=0;
  if(tx) ofx=1;
  DSPFLOAT lineStart=start;
  switch (colorLine)
    {
    case YLINEODD:
    case YLINEEVEN:
      lineStart+=bp;
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
//          pixelPositionTable[i]=(unsigned int)round(markerFloat+(((float)(i+ofx)*visibleLineLength*2)/activeSSTVParam->numberOfPixels));
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*2*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    case REDLINE:
    case BLUELINE:
      lineStart+=bp+blank+visibleLineLength*2;
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
//          pixelPositionTable[i]=(unsigned int)round(markerFloat+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    }
}

// send:
// 0 - YLine odd
// 1 - 1500 Hz
// 2 - 1900 Hz
// 3 - Red Line
// 4 - fp (1500 Hz)
// 5 - sync (1200 Hz)
// 6 - bp (1500 Hz)
// 7 - YLine even
// 8 - 2300 Hz
// 9 - 1900 Hz
//10 - Red Line
//11 - fp (1500 Hz)
//12 - sync (1200 Hz)
//13 - bp (1500 Hz)


modeBase::embState modeRobot1::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
  switch(subLine)
    {
    case 0:
      calcPixelPositionTable(YLINEODD,true);
      pixelArrayPtr=yArrayPtr;
      return MBPIXELS;
    case 1:
      txFreq=lowerFreq; // 1500 Hz for ROobot 36
      txDur=(unsigned int)rint((2*blank)/3);
      return MBTXGAP;
    case 2:
      txFreq=1900.;
      txDur=(unsigned int)rint(blank/3);
      return MBTXGAP;
    case 3:
      calcPixelPositionTable(REDLINE,true);
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 4:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(fp);
      return MBTXGAP;
    case 5:
      txFreq=syncFreq;
      txDur=(unsigned int)rint(syncDuration);
      return MBTXGAP;
    case 6:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(bp);
      lineCounter++;
      return MBTXGAP;
    case 7:
      calcPixelPositionTable(YLINEEVEN,true);
      pixelArrayPtr=yArrayPtr;
      return MBPIXELS;
    case 8:
      txFreq=2300.;
      txDur=(unsigned int)rint((2*blank)/3);
      return MBTXGAP;
    case 9:
      txFreq=1900.;
      txDur=(unsigned int)rint(blank/3);
      return MBTXGAP;
    case 10:
      calcPixelPositionTable(BLUELINE,true);
      pixelArrayPtr=blueArrayPtr;
      return MBPIXELS;
    case 11:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(fp);
      return MBTXGAP;
    case 12:
      txFreq=syncFreq;
      txDur=(unsigned int)rint(syncDuration);
      return MBTXGAP;
    case 13:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(bp);
      return MBTXGAP;
    default:
      return MBENDOFLINE;
    }
}

void modeRobot1::getLine()
{
  getLineY(true);
}

