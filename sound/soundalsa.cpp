#include "soundalsa.h"
#include "configparams.h"


soundAlsa::soundAlsa()
{
  captureHandle=NULL;
  playbackHandle=NULL;
}

soundAlsa::~soundAlsa()
{

}

void soundAlsa::closeDevices()
{
  if(captureHandle!=NULL) snd_pcm_close(captureHandle);
  captureHandle=NULL;
  if(playbackHandle!=NULL) snd_pcm_close(playbackHandle);
  playbackHandle=NULL;

}

bool soundAlsa::init(int samplerate)
{
  int iteration=0;
  soundDriverOK=false;
  sampleRate=samplerate;
  int err;
  QString tempDevice;
  tempDevice=outputAudioDevice.left(outputAudioDevice.indexOf(" "));
  for(iteration=0;iteration<20;iteration++)
    {
      err = snd_pcm_open(&playbackHandle,tempDevice.toLatin1().data(), SND_PCM_STREAM_PLAYBACK,0); //open in blocking mode
      if (err==-EBUSY)
        {
          msleep(500);
          continue;// give it another try
        }
      else
        {
          if(!alsaErrorHandler(err,"Unable to open "+outputAudioDevice)) return false;
          break;
        }
    }
  if(!alsaErrorHandler(err,"Unable to open "+outputAudioDevice)) return false;
  tempDevice=inputAudioDevice.left(inputAudioDevice.indexOf(" "));
  err = snd_pcm_open(&captureHandle,tempDevice.toLatin1().data(), SND_PCM_STREAM_CAPTURE, 0);
  if(!alsaErrorHandler(err,"Unable to open "+inputAudioDevice)) return false;
  snd_pcm_hw_params_malloc ( &hwparams );
  snd_pcm_sw_params_malloc ( &swparams );

  if(setupSoundParams(true))
    {
      if(setupSoundParams(false)) soundDriverOK=true;
    }
  snd_pcm_hw_params_free ( hwparams );
  snd_pcm_sw_params_free ( swparams );
  return soundDriverOK;
}

void soundAlsa::prepareCapture()
{
  int err;
  if(!soundDriverOK) return;
  if((err=snd_pcm_prepare (captureHandle)) < 0)
    {
      alsaErrorHandler(err,"Unable to prepare "+inputAudioDevice);
    }
  if((err=snd_pcm_start (captureHandle))< 0)
    {
      alsaErrorHandler(err,"Unable to start "+inputAudioDevice);
    }
}

void soundAlsa::preparePlayback()
{
  if(!soundDriverOK) return;
  snd_pcm_prepare (playbackHandle);
}

int soundAlsa::read(int &countAvailable)
{
  int i,count;
  if(!soundDriverOK) return 0;
  //  addToLog("1",LOGPERFORM);
  countAvailable=snd_pcm_avail(captureHandle); // check for available frames
  if(countAvailable>=DOWNSAMPLESIZE)
    {
      addToLog(QString("countAV %1").arg(countAvailable),LOGPERFORM);
      count = snd_pcm_readi(captureHandle, tempRXBuffer,DOWNSAMPLESIZE);
      addToLog(QString("3 %1").arg(count),LOGPERFORM);
      //we have enough data to read;
      if ( count < 0 )
        {
          if ( count != -EAGAIN )
            {
              if ( count == -EPIPE )
                {
                  // Overrun
                  snd_pcm_prepare (captureHandle );
                  snd_pcm_start (captureHandle);
                  errorOut() << "Overrun";
                }
              else
                {
                  snd_pcm_drop (captureHandle );
                  errorOut() <<"Overrun , reason: "<< count << "Stopping device";
                }
            }
          addToLog("soundIO: sound eagain",LOGSOUND);
          return 0;
        }
      if(count!=DOWNSAMPLESIZE)
        {
          errorOut() <<"Partial read "<< count ;
          return 0;
        }

      if (is32bit)
        {
          for(i=1;i<DOWNSAMPLESIZE;i++)
            {
              tempRXBuffer[i]=tempRXBuffer[2*i+1];
            }
        }
      if (isStereo)
        {
          for(i=1;i<DOWNSAMPLESIZE;i++)
            {
              tempRXBuffer[i]=tempRXBuffer[2*i];
            }
        }
      return DOWNSAMPLESIZE;
    }
  return 0;
}

int soundAlsa::write(uint numFrames)
{
  int error,framesWritten;
  if(!soundDriverOK) return 0;
  snd_pcm_sframes_t availp;
  snd_pcm_sframes_t delayp;
  if((framesWritten =  snd_pcm_writei ( playbackHandle, tempTXBuffer, numFrames))<0)
    {
      alsaErrorHandler(framesWritten,"Error in writing to "+inputAudioDevice);
      if ( framesWritten ==  -EAGAIN )
        {
          return -1;
        }
      else if ( framesWritten == -EPIPE )
        {
          /* underrun */
          error = snd_pcm_prepare (playbackHandle);
          if ( error < 0 )
            {
              alsaErrorHandler(framesWritten,"Can't recover from underrun, prepare failed");
              snd_pcm_drop (playbackHandle);
            }
        }
      else
        {
          alsaErrorHandler(framesWritten,"Unhandled error in playback");
          snd_pcm_drop (playbackHandle );
        }
      return -1;
    }
  if(framesWritten!=(int)numFrames)
    {
      errorHandler("Sound write error",QString("Frames written = %1").arg(framesWritten));
    }
  snd_pcm_avail_delay 	( playbackHandle,&availp,&delayp);
  addToLog(QString("latency %1 %2").arg(availp).arg(delayp),LOGSOUND);
  return framesWritten;
}

void soundAlsa::waitPlaybackEnd()
{

  //  int i;
  //  snd_pcm_sframes_t availp;
  //  snd_pcm_sframes_t delayp;
  if(!soundDriverOK) return;

  addToLog("waitPlaybackend",LOGSOUND);
  snd_pcm_drain(playbackHandle);
  addToLog("drain end",LOGSOUND);
  //  for(i=0;i<20;i++)
  //  {
  //  snd_pcm_avail_delay 	( playbackHandle,&availp,&delayp);
  //  addToLog(QString("latency%1 %2 %3").arg(i+1).arg(availp).arg(delayp),LOGSOUND);
  //  msleep(200);
  //  }
}



void soundAlsa::flushCapture()
{
  int countAvailable,count;
  if(!soundDriverOK) return;
  snd_pcm_readi(captureHandle, tempRXBuffer,DOWNSAMPLESIZE); // dummy read
  countAvailable=snd_pcm_avail(captureHandle); // check for available frames
  //  addToLog(QString("counts available %1").arg(countAvailable),LOGPERFORM);
  while(countAvailable>=DOWNSAMPLESIZE)
    {
      count=snd_pcm_readi(captureHandle, tempRXBuffer,DOWNSAMPLESIZE);
      countAvailable-=count;
    }
  //  countAvailable=snd_pcm_avail(captureHandle); // check for available frames
  //  addToLog(QString("counts available %1").arg(countAvailable),LOGPERFORM);
}

void soundAlsa::flushPlayback()
{

}

bool soundAlsa::setupSoundParams(bool isCapture)
{
  int err=0;
  int dir=0;
  snd_pcm_t *handle;
  isStereo=false;
  snd_pcm_format_t  format;

  playbackPeriodSize=PERIODSIZE;
  playbackBufferSize=BUFFERSIZE;
  capturePeriodSize=PERIODSIZE;
  captureBufferSize=PERIODSIZE*2;


  if(isCapture) handle=captureHandle;
  else handle=playbackHandle;

  /* Choose all parameters */
  err = snd_pcm_hw_params_any ( handle, hwparams );
  if(!alsaErrorHandler(err,"Broken configuration : no configurations available"))
    {
      return false;
    }
  err=snd_pcm_hw_params_get_format (hwparams, &format );


  /* Set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access ( handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED );
  if(!alsaErrorHandler(err,"Access type not available : ")) return false;

  /* Set the sample format */
  is32bit=false;
  err = snd_pcm_hw_params_set_format ( handle, hwparams, SND_PCM_FORMAT_S16_LE );
  if(!alsaErrorHandler(err,"Sample format S16_LE not available : "))
    {
      is32bit=true;
      err = snd_pcm_hw_params_set_format ( handle, hwparams, SND_PCM_FORMAT_S32_LE );
      if(!alsaErrorHandler(err,"Sample format S32_LE not available : "))
        {
          return false;
        }
    }
  /* Set the count of channels */
  if(isCapture)
    {
      snd_pcm_hw_params_get_channels_min(hwparams,&minChannelsCapture);
      snd_pcm_hw_params_get_channels_max(hwparams,&maxChannelsCapture);
      err = snd_pcm_hw_params_set_channels ( handle, hwparams, minChannelsCapture);
      if(!alsaErrorHandler(err,"Channels count not correct; " )) return false;
    }
  else
    {

      snd_pcm_hw_params_get_channels_min(hwparams,&minChannelsPlayback);
      snd_pcm_hw_params_get_channels_max(hwparams,&maxChannelsPlayback);
      err = snd_pcm_hw_params_set_channels ( handle, hwparams, 2); //allways stereo output
      if(!alsaErrorHandler(err,"Channels count not correct; " ))
        {
          return false;
        }
    }

  err = snd_pcm_hw_params_set_rate ( handle, hwparams, sampleRate, 0 );
  if(!alsaErrorHandler(err,QString("Samplerate %1 not available").arg(sampleRate))) return false;

  if(isCapture)
    {
      err = snd_pcm_hw_params_set_period_size_near ( handle, hwparams, &capturePeriodSize, &dir );
      if(!alsaErrorHandler(err,QString("Unable to set period size %1 for capture").arg(capturePeriodSize))) return false;
      err = snd_pcm_hw_params_set_buffer_size_near ( handle, hwparams, &captureBufferSize );
      if(!alsaErrorHandler(err,QString("Unable to set buffersize %1 for capture").arg(captureBufferSize))) return false;
    }
  else
    {
      err = snd_pcm_hw_params_set_period_size_near ( handle, hwparams, &playbackPeriodSize, &dir );
      if(!alsaErrorHandler(err,QString("Unable to set period size %1 for playback").arg(playbackPeriodSize))) return false;
      err = snd_pcm_hw_params_set_buffer_size_near ( handle, hwparams, &playbackBufferSize );
      if(!alsaErrorHandler(err,QString("Unable to set buffersize %1 for playback").arg(playbackBufferSize))) return false;
    }
  err = snd_pcm_hw_params ( handle, hwparams );
  if(isCapture)
    {
      if(!alsaErrorHandler(err,QString("Unable to set hw params for capture:"))) return false;
    }
  else
    {
      if(!alsaErrorHandler(err,QString("Unable to set hw params for playback:"))) return false;
    }

  /* Get the current swparams */
  err = snd_pcm_sw_params_current ( handle, swparams );
  if(!alsaErrorHandler(err,"Unable to determine current swparams")) return false;
  err = snd_pcm_sw_params_set_start_threshold ( handle, swparams, 128);
  if(!alsaErrorHandler(err,"Unable to set start threshold mode")) return false;
  /* Write the parameters to the record/playback device */
  err = snd_pcm_sw_params ( handle, swparams );
  if(!alsaErrorHandler(err,"Unable to set sw params for output")) return false;
  if(minChannelsCapture==STEREOCHANNEL) isStereo=true;
  return true;
}

bool soundAlsa::alsaErrorHandler(int err,QString info)
{
  if(err<0)
    {
      errorHandler(info,QString ( snd_strerror ( err )));
      return false;
    }
  return true;
}


void getCardList(QStringList &alsaInputList, QStringList &alsaOutputList)
{
  bool isOutput,isInput;
  QString deviceName;
  QString deviceDescription;
  void **hints, **n;
  char *name, *descr,  *io;

  alsaInputList.clear();
  alsaOutputList.clear();


  if (snd_device_name_hint(-1, "pcm", &hints) < 0)  return;
  n = hints;
  while (*n != NULL)
    {
      isInput=isOutput=true;
      io    = snd_device_name_get_hint(*n, "IOID");
      if(io!=NULL)
        {
          if (strcmp(io, "Input") == 0) isOutput=false;
          if (strcmp(io, "Output") == 0) isInput=false;
        }
      name   = snd_device_name_get_hint(*n, "NAME");
      descr  = snd_device_name_get_hint(*n, "DESC");
      deviceName=QString(name);
      if (descr!=NULL)
        {
          deviceDescription=QString(descr).split("\n").at(0);
        }

      if(
         !deviceName.contains("surround",Qt::CaseInsensitive)
         && !deviceName.contains("dmix",Qt::CaseInsensitive)
         && !deviceName.contains("front",Qt::CaseInsensitive)
         && !deviceName.contains("plughw",Qt::CaseInsensitive)
         && !deviceName.contains("null",Qt::CaseInsensitive)
         && !deviceName.contains("hdmi",Qt::CaseInsensitive)
         && !deviceName.contains("sysdefault",Qt::CaseInsensitive)
         && !deviceName.contains("dsnoop",Qt::CaseInsensitive)


         )
        {
          if(isInput)  alsaInputList.append(deviceName + " -- " +deviceDescription);
          if(isOutput) alsaOutputList.append(deviceName+ " -- " +deviceDescription);
        }

      if (name != NULL)  free(name);
      if (descr != NULL) free(descr);
      if (io != NULL)    free(io);
      n++;
    }
  snd_device_name_free_hint(hints);
}

