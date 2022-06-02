#ifndef SOUNDPULSE_H
#define SOUNDPULSE_H


#include "soundbase.h"

#define CAPTFRAGSIZE 512
//#define PLAYLENGTH   512
#define PLAYLENGTH   (4096*5)


#include <pulse/simple.h>
#include <pulse/error.h>

class soundPulse:public soundBase
{
public:
  soundPulse();
  ~soundPulse();
  bool init(int samplerate);
  int read(int &countAvailable);
  int write(uint numFrames);
protected:
  void flushCapture();
  void flushPlayback();
  void closeDevices();
  void waitPlaybackEnd();
private:
  struct stream_data
  {
    pa_simple*	stream;
    pa_sample_spec	stream_params;
    pa_buffer_attr  buffer_attrs;
    pa_stream_direction_t dir;
    double		src_ratio;
    size_t		blocksize;
  } sd[2];




};
#endif // SOUNDPULSE_H
