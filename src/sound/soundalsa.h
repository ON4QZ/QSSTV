#ifndef SOUNDALSA_H
#define SOUNDALSA_H


#include "soundbase.h"
#include <alsa/asoundlib.h>

void getCardList(QStringList &alsaInputList, QStringList &alsaOutputList);

class soundAlsa:public soundBase
{

public:
  soundAlsa();
  ~soundAlsa();
  bool init(int samplerate);
  void prepareCapture();
  void preparePlayback();
  int read(int &countAvailable);
  int write(uint numFrames);

protected:
  void flushCapture();
  void flushPlayback();
  void closeDevices();
  void waitPlaybackEnd();

private:
  bool setupSoundParams(bool isCapture);
  bool alsaErrorHandler(int err,QString Info);
  snd_pcm_uframes_t playbackPeriodSize;
  snd_pcm_uframes_t playbackBufferSize;
  snd_pcm_uframes_t capturePeriodSize;
  snd_pcm_uframes_t captureBufferSize;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  snd_pcm_t      *playbackHandle;
  snd_pcm_t      *captureHandle;
  unsigned int minChannelsCapture;
  unsigned int maxChannelsCapture;
  unsigned int minChannelsPlayback;
  unsigned int maxChannelsPlayback;
  bool is32bit;
};

#endif // SOUNDALSA_H
