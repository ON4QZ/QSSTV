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
#include "modegbr2.h"

// Scottie 1,2 and SDX

modeGBR2::modeGBR2(esstvMode m,unsigned int len, bool tx,bool narrowMode):modeBase(m,len,tx,narrowMode)
{
}


modeGBR2::~modeGBR2()
{
}

void modeGBR2::setupParams(double clock)
{
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-2*blank-syncDuration)/3.;
}

modeBase::embState modeGBR2::rxSetupLine()
{
  // the start of the Scottie mode is always at the start of the green line

  // if(subLine==0) addToLog(QString("modeGBR2: subLine %1, line=%2").arg(subLine).arg(lineCounter),DBMODES);
  switch(subLine)
    {
    case 0:
      debugState=stG1;
      start=lineTimeTableRX[lineCounter];
      markerFloat=start+blank;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 1:
      calcPixelPositionTable(GREENLINE,false);
      markerFloat+=visibleLineLength;
      debugState=stColorLine0;
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 2:
      debugState=stG2;
      markerFloat+=blank;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 3:
      calcPixelPositionTable(BLUELINE,false);
      markerFloat+=visibleLineLength;
      debugState=stColorLine1;
      pixelArrayPtr=blueArrayPtr;
      return MBPIXELS;
    case 4:
      debugState=stFP;
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 5:
      debugState=stSync;
      markerFloat+=syncDuration;
      marker=(unsigned int)round(markerFloat);
      syncPosition=marker;
      return MBSYNC;
    case 6:
      debugState=stBP;
      markerFloat+=bp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 7:
      calcPixelPositionTable(REDLINE,false);
      markerFloat+=visibleLineLength;
      debugState=stColorLine2;
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
      break;
    default:
      return MBENDOFLINE;
    }
}

void modeGBR2::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;
  DSPFLOAT lineStart=start;
  int ofx=0;
  if(tx) ofx=1;
//  debugState=colorLine;
  switch (colorLine)
    {
    case GREENLINE:
      lineStart+=blank;
      // addToLog(QString("gbr2: greenstart=%1").arg(start),DBMODES);

      break;
    case BLUELINE:
      lineStart+=(blank+visibleLineLength+blank);
      syncEndPosition=(unsigned int)(lineStart+fp+syncDuration);

      // addToLog(QString("gbr2: bluestart=%1").arg(start),DBMODES);

      break;
    case REDLINE:
      lineStart+=2*blank+fp+syncDuration+bp+2.*visibleLineLength;
      // addToLog(QString("gbr2: redstart=%1").arg(start),DBMODES);

      break;
    }
  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
    }
}


unsigned long modeGBR2::adjustSyncPosition(unsigned long syncPos0,bool isRetrace)
{
#define SCOTTYCOMPENSATION 15
  if(isRetrace) return syncPos0+SCOTTYCOMPENSATION;
  if(syncPos0<(unsigned long)(fp+2*visibleLineLength+2*blank+syncDuration/2))
    {
      return syncPos0+(unsigned long)(bp+visibleLineLength)+SCOTTYCOMPENSATION;
    }
  else
    {
      return syncPos0-(unsigned long)(fp+2*visibleLineLength+2*blank+syncDuration)+SCOTTYCOMPENSATION;
    }

}

//unsigned long modeGBR2::retraceAdjust(unsigned long syncPos)
//{
//  return syncPos-(unsigned long)(blank+visibleLineLength+syncDuration);
//}

modeBase::embState modeGBR2::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
  // if(subLine==0) addToLog(QString("modeGBR2: subLine %1, line=%2").arg(subLine).arg(lineCounter),DBMODES);
  switch(subLine)
    {
    case 0:
      calcPixelPositionTable(GREENLINE,true);
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 1:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(blank);
      return MBTXGAP;
    case 2:
      calcPixelPositionTable(BLUELINE,true);
      pixelArrayPtr=blueArrayPtr;
      return MBPIXELS;
    case 3:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(fp);
      return MBTXGAP;
    case 4:
      txFreq=syncFreq;
      txDur=(unsigned int)rint(syncDuration);
      return MBTXGAP;
    case 5:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(bp);
      return MBTXGAP;
    case 6:
      calcPixelPositionTable(REDLINE,true);
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 7:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(blank);
      return MBTXGAP;
    default:
      return MBENDOFLINE;
    }
}
