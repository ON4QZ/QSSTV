#ifndef DRMSTATUSFRAME_H
#define DRMSTATUSFRAME_H
#include "drm.h"

#include <QFrame>
#include <QPixmap>

class demodulator;

namespace Ui {
class drmStatusFrame;
}

class drmStatusFrame : public QFrame
{
  Q_OBJECT
  
public:
  explicit drmStatusFrame(QWidget *parent = 0);
  ~drmStatusFrame();
  void init();
  void setStatus();

private:
  Ui::drmStatusFrame *ui;
  QString call;
//  void paintEvent (QPaintEvent *e);
  QString mode;
  float bandwidth;
  QString interleave;
  QString protection;
  int qam;

  QPixmap *greenPXM;
  QPixmap *redPXM;
  QPixmap *yellowPXM;

  bool prevTimeSync;
  int prevFacValid;
  bool prevFrameSync;
  emscStatus prevMscValid;
  int prevCurrentSegment;
  uint prevTransportID;
  int prevRxSeg;
  int prevRobustnessMode;
  QString prevCall;
  int prevSspectrumOccupancy;
  int prevProtection;
  int prevInterleaverDepth;
  int prevMscMode;
  int prevBodyTotalSegments;
  float prevWMERFAC;
  int prevFreqOff;
  int prevBlockCount;
  int drmBusyCount;
};

QString modeToString(uint mode);
QString compactModeToString(uint mode);


#endif // DRMSTATUSFRAME_H
