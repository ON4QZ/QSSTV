#ifndef DRMRX_H
#define DRMRX_H

#include <QObject>
#include "appdefs.h"
#include "drmdefs.h"
#include "filters.h"


class drmRx : public QObject
{
  Q_OBJECT
public:
  explicit drmRx(QObject *parent = 0);
  ~drmRx();
  void init();
  void run(DSPFLOAT *dataPtr);
  void eraseImage(){}

signals:

public slots:
  private:
  int n,im;
  float rRation;
  float resamp_signal[2 * DRMBUFSIZE];
  drmHilbertFilter iqFilter;
};

#endif // DRMRX_H
