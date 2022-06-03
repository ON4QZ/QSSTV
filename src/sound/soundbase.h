#ifndef SOUNDBASE_H
#define SOUNDBASE_H

#include "appglobal.h"
#include "wavio.h"
#include "buffermanag.h"
#include "downsamplefilter.h"


#include <QThread>
#include <QMutex>

#define BYTESPOWER 18


//WWV WWVH             2500.0, 5000.0, 10000.0

//·      GBR                           60.0 kHz

//·      RWM                          4996.0, 9996.0, 14996.0

//·      CHU                            7335.0



#define PERIODSIZE (DOWNSAMPLESIZE)
#define BUFFERSIZE (8*DOWNSAMPLESIZE)
#define CALIBRATIONSIZE (PERIODSIZE)
#define CALIBRATIONLEADIN 80


class soundBase : public QThread
{
  Q_OBJECT

public:
  enum edataSrc{SNDINCARD,SNDINFROMFILE,SNDINCARDTOFILE};
  enum edataDst{SNDOUTCARD,SNDOUTTOFILE};
  enum eplaybackState{PBINIT,PBSTARTING,PBRUNNING,PBCALIBRATESTART,PBCALIBRATEWAIT,PBCALIBRATE,PBEND};
  enum ecaptureState{CPINIT,CPSTARTING,CPRUNNING,CPCALIBRATESTART,CPCALIBRATEWAIT,CPCALIBRATE,CPEND};

  explicit soundBase(QObject *parent = 0);
  ~soundBase();
  virtual bool init(int samplerate)=0;
  void run();
  void idleRX();
  void idleTX();
  void stopSoundThread();
  virtual void getCardList() {;}

  bool startCapture();
  bool startPlayback();
  buffer<FILTERPARAMTYPE,BYTESPOWER> rxBuffer;
  buffer<FILTERPARAMTYPE,BYTESPOWER> rxVolumeBuffer;
  buffer<SOUNDFRAME,16> txBuffer;
  double getVolumeDb(){return volume;}
  FILTERPARAMTYPE *getVolumePtr() {return downsampleFilterPtr->getVolumePtr();}
  const QString getLastError() { return lastErrorStr;}
  bool isPlaying() {return playbackState!=PBINIT;}
  bool isCapturing() {return captureState!=CPINIT;}

  bool calibrate(bool isCapture);
  bool calibrationCount(unsigned int &frames, double &elapsedTime);
  int countAvailable;
signals:

public slots:

protected:
  bool soundDriverOK;
  bool isStereo;
  int capture();
  int play();

  virtual int read(int &countAvailable)=0;
  virtual int write(uint numFrames)=0;
  virtual void flushCapture()=0;
  virtual void flushPlayback()=0;
  virtual void prepareCapture() {;}
  virtual void preparePlayback() {;}
  virtual void closeDevices()=0;
  virtual void waitPlaybackEnd()=0;


  int sampleRate;
  qint16 tempRXBuffer[DOWNSAMPLESIZE*2*2]; // in some cases the hardware interface is stereo (can be S16_LE or S32_LE)
  quint32 tempTXBuffer[DOWNSAMPLESIZE*2];
  bool stopThread;
  eplaybackState playbackState;
  ecaptureState  captureState;


  wavIO waveIn;
  wavIO waveOut;
  void errorHandler(QString title,QString info);
  void switchCaptureState(ecaptureState cs);
  void switchPlaybackState(eplaybackState ps);



private:
  downsampleFilter *downsampleFilterPtr;
  double volume;
//  uint intVolume;
  int captureCalibration(bool leadIn);
  int playbackCalibration(bool leadIn);
  QMutex mutex;
  QElapsedTimer stopwatch;
  unsigned int calibrationFrames;
  unsigned int leadInCounter;
  int calibrationTime;
  double ucalibrationTime;
  double ustartcalibrationTime;
  struct timespec ts;
  QString lastErrorStr;
  quint64 storedFrames;
  bool prebuf;
  unsigned int prevFrames;


};

#endif // SOUNDBASE_H
