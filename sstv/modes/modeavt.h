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
#ifndef MODEAVT_H
#define MODEAVT_H

#include "modebase.h"
enum eTrailerState {D1900,D1900END,DELAYHALF,DELAYFULL,BITS,CALCDELAY,WAITSTART};

/**
	@author Johan Maes <on4qz@telenet.be>
*/
class modeAVT : public modeBase
{

public:
  modeAVT(esstvMode m,unsigned int len,bool tx,bool narrowMode);
  ~modeAVT();
  eModeBase process(quint16 *demod, unsigned int, bool, unsigned int rxPos);
protected:
  embState rxSetupLine();
	void calcPixelPositionTable(unsigned int colorLine,bool tx);
	void setupParams(double clock);
  embState txSetupLine();
  bool avtTrailerDetect;
  unsigned int duration;
  unsigned int bitCounter;
  unsigned int code;
  unsigned int count;
  eTrailerState trailerState;
  DSPFLOAT avgSample;
  unsigned int delay;
private:
  void switchTrailerState(eTrailerState newState);
};


#endif
