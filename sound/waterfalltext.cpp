#include "waterfalltext.h"
#include "appglobal.h"
#include "configparams.h"
#include "imageviewer.h"

#include "filters.h"
#include "supportfunctions.h"
#include "drm.h"

#include "math.h"

#include <QPainter>
#include <QDebug>

//#define FREQ_AMPLITUDE 16E3
#define FREQ_AMPLITUDE 6E3
#define FREQ_OFFSET 300.0
#define FREQ_MAX 2600.


waterfallText::waterfallText()
{
  out=NULL;
  outFiltered=NULL;
  dataBuffer=NULL;
  txFilter=NULL;
  phr=phi=NULL;
}

waterfallText::~waterfallText()
{
  fftw_destroy_plan(plan);
  if(out) fftw_free(out);
  if(outFiltered) delete outFiltered;
  if(dataBuffer) fftw_free(dataBuffer);
}

void waterfallText::init()
{
  int i;
  double ph;
  double binSize;
  if(phr!=NULL) delete phr;
  if(phi!=NULL) delete phi;
  fftLength=TXSTRIPE*SUBSAMPLINGFACTOR/2;
  samplingrate=BASESAMPLERATE;
  binSize=(double)(BASESAMPLERATE)/((double)fftLength);
  txFilter= new wfFilter(TXSTRIPE);
  out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*fftLength);
  dataBuffer = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*fftLength);
  outFiltered = new DSPFLOAT [fftLength];
  audioBuf = new DSPFLOAT [fftLength];
  // create the fftw plan
  addToLog("fftw_plan waterfall start",LOGFFT);
  plan = fftw_plan_dft_1d(fftLength, dataBuffer, out, FFTW_BACKWARD, FFTW_ESTIMATE);
  addToLog("fftw_plan waterfall stop",LOGFFT);
  imageWidth=(FREQ_MAX-FREQ_OFFSET)/binSize;

  //  imageWidth=200;
  startFreqIndex=(int)round(FREQ_OFFSET/binSize);

  //Chirp
  phr=new double[imageWidth];
  phi=new double[imageWidth];
  amplitude=FREQ_AMPLITUDE/sqrt(imageWidth);
  for(i=0;i<imageWidth;i++)
  {
    ph=(-M_PI/imageWidth)*i*i;
    phr[i]=amplitude*cos(ph);
    phi[i]=amplitude*sin(ph);
  }



}

double waterfallText::getDuration(QString txt)
{
  if(!txt.isNull())
    {
      setupImage(convert(txt));
    }
  return ((double)(line*3*fftLength))/(double)samplingrate;
}

void waterfallText::setText(QString txt)
{
  QString t=convert(txt);
  setupImage(t);
}


DSPFLOAT * waterfallText::nextLine()
{
  QRgb *cPtr;
  int i,freqIndex;

  if(dLine%3==0)
  {
    line--;
    if(line<0)
    {
      return NULL;
    }

    addToLog(QString("sendingline %1").arg(line),LOGSYNTHES);
    cPtr=(QRgb *)image.scanLine(line);
    for(i=0;i<fftLength;i++)
    {
      dataBuffer[i][0]=0.0;
      dataBuffer[i][1]=0.0;
    }

    for(i=0;i<imageWidth;i++)
    {
      freqIndex=i+startFreqIndex;
      if((cPtr[i]&0xffffff)!=0)
      {
        dataBuffer[freqIndex][0]= phr[i];
        dataBuffer[freqIndex][1]= phi[i];
      }
    }
    fftw_execute(plan);
    for(i=0;i<fftLength;i++)
    {
      outFiltered[i]=(DSPFLOAT) out[i][0];
    }
  }
  dLine++;
  return outFiltered;
}



void waterfallText::setupImage(QString txt)
{
  QRect rct;
  QColor c;
  QPainter p;
  QPen pen;
  pen.setColor(Qt::white);
  dLine=0;
  image=QImage(QSize(imageWidth,80),QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::black);
  p.begin(&image);
  p.setPen(pen);
  if(wfBold) p.setFont(QFont(wfFont,wfFontSize,QFont::Bold));
  else p.setFont(QFont(wfFont,wfFontSize,QFont::Light));
  rct=p.boundingRect(QRect(0,0,imageWidth,30),Qt::AlignTop|Qt::AlignCenter,txt);
  p.end();
  height=rct.height();
  width=imageWidth;
  image=QImage(QSize(width,height),QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::black);
  p.begin(&image);
  p.setPen(pen);
  if(wfBold) p.setFont(QFont(wfFont,wfFontSize,QFont::Bold));
  else p.setFont(QFont(wfFont,wfFontSize,QFont::Light));
  p.drawText(QRectF(0,0,width,height),Qt::AlignCenter,txt);
  p.end();
  line=image.height();
}


QString  waterfallText::convert(QString txt)
{
  mexp.clear();
  mexp.addConversion('m',myCallsign);
  mexp.addConversion('s',QString::number(lastAvgSNR,'g',2));
  mexp.addConversion('c',lastReceivedCall);

  QString t=mexp.convert(txt);
  return t;
}
