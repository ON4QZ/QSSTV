/***************************************************************************
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
#include "modegbr.h"


// used with Martin 1 and Martin 2

modeGBR::modeGBR(esstvMode m,unsigned int len, bool tx,bool narrowMode):modeBase(m,len,tx,narrowMode)
{

}


modeGBR::~modeGBR()
{
}


void modeGBR::setupParams(double clock)
{
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-2*blank-syncDuration)/3.;
}

modeBase::embState modeGBR::rxSetupLine()
{	
  start=lineTimeTableRX[lineCounter];
  //if(subLine==0) addToLog(QString("modeGBR: subLine %1, line=%2, absSampleCounter %3").arg(subLine).arg(lineCounter).arg(start+rxSampleCounter),DBMODES);

  switch(subLine)
    {
    case 0:
      debugState=stBP;
      start=lineTimeTableRX[lineCounter];
      markerFloat=start+bp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 1:
      calcPixelPositionTable(GREENLINE,false);
      markerFloat+=visibleLineLength;
      debugState=stColorLine0;
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 2:
      debugState=stG1;
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
      debugState=stG2;
      markerFloat+=blank;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 5:
      calcPixelPositionTable(REDLINE,false);
      markerFloat+=visibleLineLength;
      debugState=stColorLine2;
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 6:
      debugState=stFP;
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 7:
      debugState=stSync;
      markerFloat+=syncDuration;
      marker=(unsigned int)round(markerFloat);
      syncPosition=marker;
      return MBSYNC;
      break;
    default:
      return MBENDOFLINE;
    }
}

void modeGBR::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;
  DSPFLOAT lineStart=start;
  int ofx=0;
  if(tx) ofx=1;
  switch (colorLine)
    {
    case GREENLINE:
      lineStart+=bp;
      //       addToLog(QString("calcPixelPosition: startGreen %1").arg(start+rxSampleCounter),DBMODES);
      break;
    case BLUELINE:
      lineStart+=(bp+visibleLineLength+blank);
      //       addToLog(QString("calcPixelPosition: startBlue %1").arg(start+rxSampleCounter),DBMODES);
      break;
    case REDLINE:
      lineStart+=(bp+2.*visibleLineLength+2.*blank);
      //        addToLog(QString("calcPixelPosition: startRed %1").arg(start+rxSampleCounter),DBMODES);
      break;
    }
  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
    }
}


modeBase::embState modeGBR::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
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
      txDur=(unsigned int)rint(blank);
      return MBTXGAP;
    case 4:
      calcPixelPositionTable(REDLINE,true);
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 5:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(fp);
      return MBTXGAP;
    case 6:
      txFreq=syncFreq;
      txDur=(unsigned int)rint(syncDuration);
      return MBTXGAP;
    case 7:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(bp);
      return MBTXGAP;
    default:
      return MBENDOFLINE;
    }
}

