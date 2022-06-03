#ifndef soundConfig_H
#define soundConfig_H

#include "baseconfig.h"
#include "soundbase.h"

extern int samplingrate;
extern double rxClock;
extern double txClock;
extern bool pulseSelected;
extern bool alsaSelected;
extern bool swapChannel;
extern bool pttToneOtherChannel;
extern QString inputAudioDevice;
extern QString outputAudioDevice;
extern soundBase::edataSrc soundRoutingInput;
extern soundBase::edataDst soundRoutingOutput;
extern quint32 recordingSize;


namespace Ui {
  class soundConfig;
  }

class soundConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit soundConfig(QWidget *parent = 0);
  ~soundConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();



private:
  Ui::soundConfig *ui;

};

#endif // soundConfig_H
