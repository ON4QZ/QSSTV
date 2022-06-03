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
#include "moderobot2.h"


modeRobot2::modeRobot2(esstvMode m,unsigned int len,bool tx,bool narrowMode): modeBase(m,len,tx,narrowMode)
{
}


modeRobot2::~modeRobot2()
{
}

void modeRobot2::setupParams(double clock)
{
  //double tmp=(activeSSTVParam->imageTime/(double)activeSSTVParam->numberOfDataLines)*clock;
  //visibleLineLength=(tmp-fp-bp-2*blank-syncDuration)/4;
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-2*blank-syncDuration)/4.;
}




modeBase::embState modeRobot2::rxSetupLine()
{
  switch(subLine)
		{
			case 0:
				debugState=stBP;
        start=lineTimeTableRX[lineCounter];
        markerFloat=start+bp;
        marker=(unsigned int)round(markerFloat);
        return MBRXWAIT;
			case 1:
				calcPixelPositionTable(YLINEODD,false);
        markerFloat+=2*visibleLineLength;
        debugState=stColorLine0;
        pixelArrayPtr=yArrayPtr;
        return MBPIXELS;
      case 2:
				debugState=stG1;
        markerFloat+=blank;
        marker=(unsigned int)round(markerFloat);
				return MBRXWAIT;
			case 3:
				calcPixelPositionTable(REDLINE,false);
        markerFloat+=visibleLineLength;
        debugState=stColorLine1;
				pixelArrayPtr=redArrayPtr;
				return MBPIXELS;
			case 4:
        debugState=stG2;
        markerFloat+=blank;
        marker=(unsigned int)round(markerFloat);
				return MBRXWAIT;
			case 5:
				calcPixelPositionTable(BLUELINE,false);
        markerFloat+=visibleLineLength;
        debugState=stColorLine2;
				pixelArrayPtr=blueArrayPtr;
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

			default:
				return MBENDOFLINE;
		}
}



void modeRobot2::showLine()
{
	yuvConversion(yArrayPtr);
}

void modeRobot2::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;
  DSPFLOAT lineStart=start;
  int ofx=0;
  if(tx) ofx=1;
//  debugState=colorLine;
  switch (colorLine)
    {
    case YLINEODD:
      lineStart+=bp;
    break;
    case REDLINE:
      lineStart+=(bp+2*visibleLineLength+blank);
      break;
    case BLUELINE:
      lineStart+=bp+2*blank+3*visibleLineLength;
      // addToLog(QString("gbr2: redstart=%1").arg(start),DBMODES);

      break;
    }
  switch (colorLine)
    {
      case YLINEODD:
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*2*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    default:
      for(i=0;i<activeSSTVParam->numberOfPixels;i++)
        {
          pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
        }
      break;
    }

}


modeBase::embState modeRobot2::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
  switch(subLine)
		{
			case 0:
        txFreq=lowerFreq;
				txDur=(unsigned int)rint(bp+6);
				return MBTXGAP;
			case 1:
				calcPixelPositionTable(YLINEODD,true);
				pixelArrayPtr=yArrayPtr;
				return MBPIXELS;
			case 2:
        txFreq=lowerFreq;
				txDur=(unsigned int)rint((2*blank)/3);
				return MBTXGAP;
			case 3:
				txFreq=1900.;
				txDur=(unsigned int)rint(blank/3);
				return MBTXGAP;
			case 4:
				calcPixelPositionTable(REDLINE,true);
				pixelArrayPtr=redArrayPtr;
				return MBPIXELS;
			case 5:
				txFreq=2300.;
				txDur=(unsigned int)rint((2*blank)/3);
				return MBTXGAP;
			case 6:
				txFreq=1900.;
				txDur=(unsigned int)rint(blank/3);
				return MBTXGAP;
			case 7:
				calcPixelPositionTable(BLUELINE,true);
				pixelArrayPtr=blueArrayPtr;
				return MBPIXELS;
			case 8:
        txFreq=lowerFreq;
				txDur=(unsigned int)rint(fp);
				return MBTXGAP;
			case 9:
        txFreq=syncFreq;
				txDur=(unsigned int)rint(syncDuration);
				return MBTXGAP;

			default:
				return MBENDOFLINE;
		}
}




void modeRobot2::getLine()
{
	getLineY(false);
}




