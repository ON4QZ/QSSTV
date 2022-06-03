/***************************************************************************
 *   Copyright (C) 2004 by Johan Maes - ON4QZ                              *
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
#ifndef SYNTHES_H
#define SYNTHES_H

#include <math.h>
#include "filterparam.h"
#include "waterfalltext.h"


#define SINTABLEN 2048
#define TONEBUFLEN (BASESAMPLERATE/1200)

class synthesizer
{
public:
	synthesizer(double txSmpClock);
	~synthesizer();
	double nextSample(double freq)
		{
  		double temp;
  		int t;
  		temp=(freq/txSamplingClock)*(double)SINTABLEN+oldAngle;
  		oldAngle=fmod(temp,SINTABLEN);
  		t=(int)(oldAngle+0.5);
      return sineTable[t%SINTABLEN];
		}
	void sendTone(double duration,double lowerFrequency,double upperFrequency, bool concat);
	void sendSamples(unsigned int numSamples,double frequency);
	void sendSweep(unsigned int duration,double lowerFrequency, double upperFrequency);
	void sendSilence(double duration);
	void sendSample(double freq);
	void setFilter(efilterType txFilterType);
  void sendWFText();
  void writeBuffer(quint32 *buffer,int len);
private:
	double txSamplingClock;
	double oldAngle;
	double sineTable[SINTABLEN];
//	const float *filterI;
//	unsigned int filterLength;
  SOUNDFRAME filter(double sample);
  void write(double sample);
	double sample;
	double adjust;
//  waterfallText *waterfallPtr;
  double toneBuffer[TONEBUFLEN];
  uint pttToneCounter;
//  void fillBuffer();  //only for test
};

extern synthesizer *synthesPtr;



#endif







