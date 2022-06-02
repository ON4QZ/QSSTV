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
#include "modepd.h"

modePD::modePD(esstvMode m,unsigned int len,bool tx,bool narrowMode): modeBase(m,len,tx,narrowMode)
{

}


modePD::~modePD()
{

}

void modePD::setupParams(double clock)
{
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-syncDuration)/4;
}


modeBase::embState modePD::rxSetupLine()
{
  start=lineTimeTableRX[lineCounter];
  switch(subLine)
    {
    case 0:
      debugState=stBP;
      start=lineTimeTableRX[lineCounter];
      markerFloat=start+bp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 1:
      debugState=stColorLine0 ;
      calcPixelPositionTable(YLINEODD,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=yArrayPtr;
      return MBPIXELS;
    case 2:
      debugState=stColorLine1 ;
      calcPixelPositionTable(REDLINE,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 3:
      debugState=stColorLine2;
      calcPixelPositionTable(BLUELINE,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=blueArrayPtr;
      return MBPIXELS;
    case 4:
      debugState=stColorLine3;
      calcPixelPositionTable(YLINEEVEN,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 5:
      debugState=stFP;
      start=lineTimeTableRX[lineCounter];
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 6:
      debugState=stSync;
      markerFloat+=syncDuration;
      marker=(unsigned int)round(markerFloat);
      syncPosition=marker;
      return MBSYNC;
    default:
      return MBENDOFLINE;
    }
}



void modePD::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;
  int ofx=0;
  if(tx) ofx=1;
  DSPFLOAT lineStart=start;
  switch (colorLine)
    {
    case YLINEODD:
      lineStart+=bp;
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    case REDLINE:
      lineStart+=(bp+visibleLineLength);
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    case BLUELINE:
      lineStart+=(bp+2*visibleLineLength);
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    case YLINEEVEN:
      lineStart+=(bp+3*visibleLineLength);
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    }
}

void modePD::showLine()
{
  yuvConversion(yArrayPtr);
  yuvConversion(greenArrayPtr);
}

modeBase::embState modePD::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
  switch(subLine)
    {
    case 0:
      calcPixelPositionTable(YLINEODD,true);
      pixelArrayPtr=yArrayPtr;
      return MBPIXELS;
    case 1:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(blank);
      return MBTXGAP;
    case 2:
      calcPixelPositionTable(REDLINE,true);
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 3:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(blank);
      return MBTXGAP;
    case 4:
      calcPixelPositionTable(BLUELINE,true);
      pixelArrayPtr=blueArrayPtr;
      return MBPIXELS;
    case 5:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(blank);
      return MBTXGAP;
    case 6:
      calcPixelPositionTable(YLINEEVEN,true);
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 7:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(fp);
      return MBTXGAP;
    case 8:
      txFreq=syncFreq;
      txDur=(unsigned int)rint(syncDuration);
      return MBTXGAP;
    case 9:
      txFreq=lowerFreq;
      txDur=(unsigned int)rint(bp);
      return MBTXGAP;
    default:
      //				lineCounter++;
      return MBENDOFLINE;
    }
}


void  modePD::getLine()
{
  getLineY(true);
}
