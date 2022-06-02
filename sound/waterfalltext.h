#ifndef WATERFALLTEXT_H
#define WATERFALLTEXT_H
#include "appdefs.h"
#include "fftw3.h"
#include "macroexpansion.h"
#include <QString>
#include <QImage>


class imageViewer;
class wfFilter;


class waterfallText
{
public:
  waterfallText();
  ~waterfallText();
  void init();
  void setText(QString txt);
  QImage *getImagePtr() {return &image;}
 DSPFLOAT *nextLine();
 int getLength() {return fftLength;}
 double getDuration(QString txt=QString());
private:
  int fftLength;
  int samplingrate;
  fftw_complex *out;
  fftw_complex *dataBuffer;

  DSPFLOAT *outFiltered;
  DSPFLOAT *audioBuf;
  fftw_plan plan;
  void setupImage(QString txt);
  QString convert(QString txt);
  int imageWidth;
  int width;
  int height;
  int line;
  wfFilter *txFilter;
  QImage image;
  int dLine;
  int startFreqIndex;
  double *phr;
  double *phi;
  double amplitude;
  macroExpansion mexp;
};

#endif // WATERFALLTEXT_H
