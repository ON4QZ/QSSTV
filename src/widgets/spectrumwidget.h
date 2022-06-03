#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QFrame>
#include "fftcalc.h"

namespace Ui {
  class spectrumWidget;
  }

class spectrumWidget : public QFrame
{
  Q_OBJECT
  
public:
  spectrumWidget(QWidget *parent = 0);
  ~spectrumWidget();
  void init(int length, int numBlocks, int isamplingrate);
//  void realFFT(short int *iBuffer);
//  void realFFT(float *iBuffer);
  void realFFT(double *iBuffer);
  void readSettings();
  void writeSettings();
  void displaySettings(bool drm);
  double getVolumeDb();
  QImage *getImage();

private slots:
  void slotMaxDbChanged(int mb);
  void slotRangeChanged(int rg);
  void getParams();
  void setParams();
  void slotAvgChanged(double);
  
private:
  Ui::spectrumWidget *ui;
  int maxdb;
  int range;
  double avg;
  fftCalc fftFunc;
};

#endif // SPECTRUMWIDGET_H
