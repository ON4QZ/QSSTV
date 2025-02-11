/**************************************************************************
*   Copyright (C) 2000-2019 by Johan Maes                                 *
*   on4qz@telenet.be                                                      *
*   https://www.qsl.net/o/on4qz                                           *
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
#include "soundpulse.h"

#include <sys/types.h>
#include <unistd.h>



soundPulse::soundPulse()
{
  sd[0].stream = NULL;
  sd[1].stream = NULL;

}

soundPulse::~soundPulse()
{
  if(sd[0].stream)  pa_simple_free(sd[0].stream);
  if(sd[1].stream)  pa_simple_free(sd[1].stream);
  sd[0].stream = NULL;
  sd[1].stream = NULL;
}


bool soundPulse::init(int samplerate)
{
  soundDriverOK=false;
  QString sname;
  sampleRate=samplerate;
  //open two streams (capture and play)
  int err;
  sd[0].stream = NULL;
  sd[0].dir = PA_STREAM_RECORD;
  sd[0].stream_params.format = PA_SAMPLE_S16LE;
  sd[0].stream_params.channels = MONOCHANNEL;
  sd[0].stream_params.rate=sampleRate;
  sd[0].buffer_attrs.maxlength = (uint32_t)-1; // let the server decide
  sd[0].buffer_attrs.minreq =    (uint32_t)-1;
  sd[0].buffer_attrs.prebuf =    (uint32_t)-1;
  sd[0].buffer_attrs.tlength =   (uint32_t)-1;
  sd[0].buffer_attrs.fragsize =  CAPTFRAGSIZE * sizeof(short int);

  sd[1].stream = NULL;
  sd[1].dir = PA_STREAM_PLAYBACK;
  sd[1].stream = NULL;
  sd[1].stream_params.format = PA_SAMPLE_S16LE;
  sd[1].stream_params.channels = STEREOCHANNEL;
  sd[1].stream_params.rate=sampleRate;
  sd[1].buffer_attrs.fragsize =  (uint32_t)-1; // let the server decide
  sd[1].buffer_attrs.maxlength = (uint32_t)-1;
  sd[1].buffer_attrs.minreq =    (uint32_t)-1;
  sd[1].buffer_attrs.prebuf =    (uint32_t)-1;
  sd[1].buffer_attrs.tlength =   PLAYLENGTH * sizeof(quint32);

  // opening device
  sname=QString("capture %1").arg(getpid());
  sd[0].stream = pa_simple_new(NULL, APPNAME.toLatin1().data(), sd[0].dir, NULL,sname.toLatin1().data(), &sd[0].stream_params, NULL,&sd[0].buffer_attrs, &err);
//  sd[0].stream = pa_simple_new(NULL, shortName.toLatin1().data(), sd[0].dir, NULL,sname.toLatin1().data(), &sd[0].stream_params, NULL,&sd[0].buffer_attrs, &err);
  if(sd[0].stream==NULL)
  {
    errorHandler("PulseAudio read init error",QString(pa_strerror(err)));
    return false;
  }

  sname=QString("playback %1").arg(getpid());
  sd[1].stream = pa_simple_new(NULL, APPNAME.toLatin1().data(), sd[1].dir, NULL,sname.toLatin1().data(), &sd[1].stream_params, NULL,&sd[1].buffer_attrs, &err);
  if(sd[1].stream==NULL)
  {
    errorHandler("PulseAudio read init error",QString(pa_strerror(err)));
    return false;
  }
  isStereo=false;
  soundDriverOK=true;
  return true;
}



int soundPulse::read(int &countAvailable)
{
  if(!soundDriverOK) return 0;
  int err=PA_OK;
  pa_usec_t latency;
  latency = pa_simple_get_latency(sd[0].stream, &err);
  if(err!=PA_OK)
  {
    errorHandler("PulseAudio latency error:", QString(pa_strerror(err)));
    return -1;
  }
  countAvailable= pa_usec_to_bytes(latency, &sd[0].stream_params);
  if(countAvailable>=DOWNSAMPLESIZE)
    {
      if (pa_simple_read(sd[0].stream, tempRXBuffer,sizeof(qint16)*DOWNSAMPLESIZE, &err) <0)
      {
        errorHandler("PulseAudio read error",QString(pa_strerror(err)));
        return -1;
      }
      else
        {
          return DOWNSAMPLESIZE;
        }
    }
  return 0;
}

int soundPulse::write(uint numFrames)
{
  if(!soundDriverOK) return 0;
  int err;
  if(numFrames!=0)
    {
      if (pa_simple_write(sd[1].stream, tempTXBuffer,sizeof(quint32)*numFrames, &err) <0)
      {
        errorHandler("PulseAudio write error",QString(pa_strerror(err)));
        return -1;
      }
    }
  return numFrames;
}

void soundPulse::waitPlaybackEnd()
{

}


void soundPulse::flushCapture()
{
  if(!soundDriverOK) return;
  int err=PA_OK;
  pa_usec_t t = pa_simple_get_latency(sd[0].stream, &err);
  if (t && err == PA_OK)
  {
    size_t bytes = pa_usec_to_bytes(t, &sd[0].stream_params);
    while (bytes > sizeof(qint16)*PERIODSIZE)
    {
      pa_simple_read(sd[0].stream, tempRXBuffer,sizeof(qint16)*PERIODSIZE, &err);
      if (err != PA_OK)
        break;
      bytes -= sizeof(qint16)*PERIODSIZE;
    }
    if (bytes)
      pa_simple_read(sd[0].stream, tempRXBuffer, bytes, &err);
  }

  //  int err;
  //  if(pa_simple_flush (sd[0].stream, &err)<0)
  //  {
  //    errorHandler("flush: ", QString(pa_strerror(err)));
  //  }
}

void soundPulse::flushPlayback()
{
  int err;
  if(!soundDriverOK) return;
  if(pa_simple_flush (sd[1].stream, &err)<0)
  {
    errorHandler("flush: ", QString(pa_strerror(err)));
  }
}

void soundPulse::closeDevices()
{
  if (sd[0].stream) pa_simple_free(sd[0].stream);
  sd[0].stream=0;
  if (sd[1].stream) pa_simple_free(sd[1].stream);
  sd[1].stream=0;
  msleep(1000);
}



