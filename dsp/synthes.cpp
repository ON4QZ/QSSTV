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
#include "synthes.h"
#include "appglobal.h"
#include "soundbase.h"
#include "soundconfig.h"

#include "supportfunctions.h"
#include <unistd.h>

/*
  To generate the frequency, we have to calculate the instant phase jump of the signal
  by dividing the frequency by the SR.
  We multiply this by the number of entries in the sine lookup table.
//  This value is added to the old index  of the sine table.
*/

synthesizer *synthesPtr;

synthesizer::synthesizer(double txSmpClock)
{
	// generate the table
	int i;
	txSamplingClock=txSmpClock;
  addToLog(QString("synthes: tx sampling clock=%1").arg(txSamplingClock),LOGSOUND);
  for (i=0;i<SINTABLEN;i++)
    {
      sineTable[i]=(sin(((double)i*M_PI*2.)/SINTABLEN)*8000.);
    }

//  waterfallPtr= new waterfallText;
  for(i=0;i<TONEBUFLEN;i++)
  {
    toneBuffer[i]=sin((i*2*M_PI)*1200./BASESAMPLERATE)*8000.;
  }
  oldAngle=0.;
  adjust=0.;
  pttToneCounter=0;
}

synthesizer::~synthesizer()
{
}

void synthesizer::sendTone(double duration,double lowerFrequency,double upperFrequency,bool concat)
{
//  fillBuffer();
  if(upperFrequency!=0)
  {
    sendSweep(duration,lowerFrequency,upperFrequency);
    return;
  }
  if(!concat) adjust=0.;
// convert duration to number of samples
  unsigned int ns=(unsigned int)((duration+adjust)*txSamplingClock+0.5);
  adjust+=duration-((double)ns)/txSamplingClock;
  sendSamples(ns,lowerFrequency);
}

void synthesizer::sendWFText()
{
  DSPFLOAT *dataPtr;
  int len;
  int i;
  len=waterfallPtr->getLength();
  while ((dataPtr=waterfallPtr->nextLine())!=NULL)
    {
      addToLog(QString("sending id len=%1").arg(len),LOGSYNTHES);
      for (i=0;i<len;i++)
        {
          write((double)dataPtr[i]);
        }
//       arrayDump(QString("wf"),dataPtr,32,true);
    }
  addToLog("end of id",LOGSYNTHES);
}

void synthesizer::sendSamples(unsigned int numSamples,double frequency)
{
	unsigned int i;
	for(i=0;i<numSamples;i++)
		{
			sendSample(frequency);
		}
}

void synthesizer::sendSweep(unsigned int duration,double lowerFrequency, double upperFrequency)
{
	unsigned int i;
	unsigned int numSamples=duration*txSamplingClock;
	double deltaFreq=(upperFrequency-lowerFrequency)/numSamples;
	for(i=0;i<numSamples;i++)
	{
		sendSample(lowerFrequency+deltaFreq*i);
	}
}

void synthesizer::sendSilence(double duration)
{
	unsigned int i;
	// convert duration to number of samples
	unsigned int ns=(uint)(duration*txSamplingClock+0.5);
	for(i=0;i<ns;i++)
		{
      write(0);
		}
}

void synthesizer::sendSample(double freq)
{
	sample=nextSample(freq);
	write(sample);
}


SOUNDFRAME synthesizer::filter(double sample)
{
 quint32 tst;
 quint32 ptt;
 tst=(quint32) round(sample);
// if(outputStereo)
//   {
//      tst+=tst<<16;
//   }
 if(pttToneOtherChannel)
   {
     ptt=((quint32)toneBuffer[(pttToneCounter++)%TONEBUFLEN])<< 16;
     tst+=ptt;
   }
 if(swapChannel)
   {
     tst=((tst>>16) & 0xFFFF)+(tst<<16);
   }
 return tst;
}

void synthesizer::write(double sample)
{
  quint32 smp=filter(sample);
//  while((!soundIOPtr->txBuffer.put(smp)) && (soundIOPtr->isPlaying()))
     while((!soundIOPtr->txBuffer.put(smp)))
    {
      usleep(2000);
    }
}



// buffer must already contain correct stereo information
void synthesizer::writeBuffer(quint32 *buffer, int len)
{
  int i;
   if(swapChannel)
     {
       for(i=0;i<len;i++)
        {
           buffer[i]=((buffer[i]>>16) & 0xFFFF)+(buffer[i]<<16);

        }
     }
  while((!soundIOPtr->txBuffer.put(buffer,len)) && (soundIOPtr->isPlaying()))
    {
      usleep(2000);
    }
}




void synthesizer::setFilter(efilterType txFilterType)
{

//  filterLength=TXNUMTAPS;
  switch (txFilterType)
    {
//      case F400:
//      case F600:
//      case F1000:
//      case F800:
      //filterI=f800TX;
      default:
      break;
    }
}

//void synthesizer::fillBuffer()
//{
//  unsigned int i;
//  unsigned int sz;
//  sz=soundIOPtr->txBuffer.getBufferSize();
//  for(i=0;i<50000;i++)
//    {
//      soundIOPtr->txBuffer.put((SOUNDFRAME)(round(nextSample(2300))));
//    }
//  for(;i<(sz);i++)
//    {
//      soundIOPtr->txBuffer.put((SOUNDFRAME)(round(nextSample(1500))));
//    }
//  for(;i<sz-1;i++)
//    {
//      soundIOPtr->txBuffer.put((SOUNDFRAME)(round(0)));
//    }
//  addToLog(QString("buffercount %1").arg(soundIOPtr->txBuffer.getWriteIndex()),LOGSYNTHES);
//}









































































