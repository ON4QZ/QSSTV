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
#include "modergb.h"

modeRGB::modeRGB(esstvMode m,unsigned int len, bool tx,bool narrowMode):modeBase(m,len,tx,narrowMode)
{
}

#define MODERGBDEBUG

modeRGB::~modeRGB()
{
}

void modeRGB::setupParams(double clock)
{
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-2*blank-syncDuration)/3.;
}




modeBase::embState modeRGB::rxSetupLine()
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
      calcPixelPositionTable(REDLINE,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=redArrayPtr;
      return MBPIXELS;
    case 2:
      debugState=stG1;
      markerFloat+=blank;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 3:
      debugState=stColorLine1;
      calcPixelPositionTable(GREENLINE,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 4:
      debugState=stG2;
      markerFloat+=blank;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 5:
      debugState=stColorLine2;
      calcPixelPositionTable(BLUELINE,false);
      markerFloat+=visibleLineLength;
      pixelArrayPtr=blueArrayPtr;
      return MBPIXELS;
    case 6:
      debugState=stFP;
      start=lineTimeTableRX[lineCounter];
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 7:
      debugState=stSync;
      markerFloat+=syncDuration;
      marker=(unsigned int)round(markerFloat);
      syncPosition=marker;
      return MBSYNC;
      default:
				return MBENDOFLINE;
		}
}

void  modeRGB::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;DSPFLOAT lineStart=start;
  int ofx=0;
  if(tx) ofx=1;
  switch (colorLine)
    {
    case REDLINE:
      lineStart+=bp;
      //       addToLog(QString("calcPixelPosition: startGreen %1").arg(start+rxSampleCounter),DBMODES);
      break;
    case GREENLINE:
      lineStart+=(bp+visibleLineLength+blank);
      //       addToLog(QString("calcPixelPosition: startBlue %1").arg(start+rxSampleCounter),DBMODES);
      break;
    case BLUELINE:
      lineStart+=(bp+2.*visibleLineLength+2.*blank);
      //        addToLog(QString("calcPixelPosition: startRed %1").arg(start+rxSampleCounter),DBMODES);
      break;
    }
  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
    }
}

//void modeRGB::calcPixelPositionTable(unsigned int colorLine,bool tx)
//{
//	unsigned int i;
//	int ofx=0;
//	if(tx) ofx=1;
//	debugState=colorLine;
//	switch (colorLine)
//		{
//			case REDLINE:
//				start+=bp;
//			break;
//			case GREENLINE:
//				start+=bp+blank+visibleLineLength;
//			break;
//			case BLUELINE:
//				start+=bp+2.*blank+2.*visibleLineLength;
//			break;
//		}
//  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
//		{
//      pixelPositionTable[i]=(unsigned int)round(start+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));

//		}
//}

modeBase::embState modeRGB::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
  switch(subLine)
		{
			case 0:
				calcPixelPositionTable(REDLINE,true);
				pixelArrayPtr=redArrayPtr;
				return MBPIXELS;
			case 1:
        txFreq=lowerFreq;
				txDur=(unsigned int)rint(blank);
				return MBTXGAP;
			case 2:
				calcPixelPositionTable(GREENLINE,true);
				pixelArrayPtr=greenArrayPtr;
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

