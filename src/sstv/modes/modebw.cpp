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
#include "modebw.h"
#include "math.h"

modeBW::modeBW(esstvMode m,unsigned int len, bool tx,bool narrowMode) : modeBase(m,len,tx,narrowMode)
{
}


modeBW::~modeBW()
{
}


void modeBW::setupParams(double clock)
{
  visibleLineLength=(getLineLength(mode,clock)-fp-bp-syncDuration);
}

modeBase::embState modeBW::rxSetupLine()
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
      calcPixelPositionTable(GREENLINE,false);
      markerFloat+=visibleLineLength;
      debugState=stColorLine0;
      pixelArrayPtr=greenArrayPtr;
      return MBPIXELS;
    case 2:
      debugState=stFP;
      markerFloat+=fp;
      marker=(unsigned int)round(markerFloat);
      return MBRXWAIT;
    case 3:
				debugState=stSync;
        syncPosition=(unsigned int)round(lineTimeTableRX[lineCounter+1]);
				return MBSYNC;
			break;
			default:
				return MBENDOFLINE;
		}
}


void modeBW::showLine()
{
	grayConversion();
}


void  modeBW::calcPixelPositionTable(unsigned int colorLine,bool tx)
{
  unsigned int i;DSPFLOAT lineStart=start;
  int ofx=0;
  if(tx) ofx=1;
  switch (colorLine)
    {
    case GREENLINE:
        lineStart+=bp;
    break;
    }
  for(i=0;i<activeSSTVParam->numberOfPixels;i++)
    {
      pixelPositionTable[i]=(unsigned int)round(lineStart+(((float)(i+ofx)*visibleLineLength)/activeSSTVParam->numberOfPixels));
    }
}

modeBase::embState modeBW::txSetupLine()
{
  start=lineTimeTableTX[lineCounter];
  switch(subLine)
		{
      case 1:
        calcPixelPositionTable(GREENLINE,true);
				pixelArrayPtr=greenArrayPtr;
				return MBPIXELS;
      case 2:
        txFreq=lowerFreq;
				txDur=(unsigned int)rint(fp);
				return MBTXGAP;
      case 3:
        txFreq=syncFreq;
				txDur=(unsigned int)rint(syncDuration);
				return MBTXGAP;
      case 0:
        txFreq=lowerFreq;
				txDur=(unsigned int)rint(bp);
				return MBTXGAP;
			default:
				return MBENDOFLINE;
		}
}

/**
	\brief get the pixel information for transmission in B&W
*/

void modeBW::getLine()
{
	getLineBW();
}

