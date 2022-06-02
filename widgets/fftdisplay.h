#ifndef FFTDISPLAY_H
#define FFTDISPLAY_H
#include <QtGui>
#include <QLabel>
#include "appdefs.h"
#include "fftw3.h"
#include <qpolygon.h>
#include <QImage>

#define FFTHIGH 2900
#define FFTLOW  200
#define FFTSPAN (FFTHIGH-FFTLOW)
#define FFTMAX 6
#define RANGE 30.

class fftDisplay : public QLabel
{
  Q_OBJECT
  
public:
  explicit fftDisplay(QWidget *parent=0);
  ~fftDisplay();
  void init(int length, int slices, int isamplingrate);
  void showFFT(double *fftData);
  void setMaxDb(int mb){fftMax=mb;}
  void setRange(int rg) {range=rg;}
  void displayWaterfall(bool wf) {showWaterfall=wf;}
  void setMarkers(int mrk1=0, int mrk2=0, int mrk3=0);
  void setAvg(double d) {avgVal=d;}
  QImage *getImage();

private:
  void drawMarkers(QPainter *painter, int top, int bot);
  void paintEvent(QPaintEvent *p);
  //  void mousePressEvent( QMouseEvent *e );
  int windowSize;
  int fftLength;
  int samplingrate;
  double *arMagSAvg;
  double *arMagWAvg;
  double maxMagnitude;
  QPolygon *fftArray;
  //  bool ready;
  bool showWaterfall;
  double fftMax;
  double range;
  QImage *imagePtr;
  QImage scaledImage;
  //  void setSize(int w,int h);
  QMutex mutex;
  int marker1;
  int marker2;
  int marker3;
  double step;  //freq step per bin
  int binBegin;
  int binEnd;
  int binDiff;
  double avgVal;
  int imWidth;
  int imHeight;
  int displayCounter;
};

#endif // FFTDISPLAY_H
