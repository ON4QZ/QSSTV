#include "fftdisplay.h"
#include "appglobal.h"
#include "configparams.h"
#include "math.h"
//#include "arraydumper.h"
#include <QPainter>


fftDisplay::fftDisplay(QWidget *parent) : QLabel(parent)
{
  //  blockIndex=0;
  arMagSAvg=NULL;
  fftArray=NULL;
  showWaterfall=false;
  fftMax=FFTMAX;
  range=RANGE;
  avgVal=0.05;
  //  plan=0;
  setScaledContents(true);
  imagePtr=NULL;
  imWidth=-1;
  imHeight=-1;
  arMagWAvg=NULL;
  displayCounter=0;
}

fftDisplay::~fftDisplay()
{
  delete imagePtr;
  if(fftArray) delete fftArray;
  if(arMagSAvg) delete []arMagSAvg;
  if(arMagWAvg) delete []arMagWAvg;
}


void fftDisplay::init(int length,int nblocks,int isamplingrate)
{
  int i;
  windowSize=length;
  fftLength=windowSize*nblocks;
  samplingrate=isamplingrate;
  if(fftArray) delete fftArray;
  if(arMagSAvg) delete [] arMagSAvg;
  step=(double)samplingrate/(double)fftLength;  //freq step per bin
  binBegin=(int) rint(FFTLOW/step);
  binEnd  =(int) rint(FFTHIGH/step);
  binDiff=binEnd-binBegin;
  fftArray=new QPolygon(binDiff);
  arMagSAvg=new double[binDiff];
  for(i=0;i<binDiff;i++)
    {
      arMagSAvg[i]=-30.;
    }
  // create the fftw plan
  //  plan = fftw_plan_r2r_1d(fftLength, dataBuffer, out, FFTW_R2HC, FFTW_ESTIMATE);
  update();
  QLabel::update();
}




void fftDisplay::setMarkers(int mrk1, int mrk2, int mrk3)
{
  marker1=mrk1;
  marker2=mrk2;
  marker3=mrk3;
  update();
}

void fftDisplay::showFFT(double *fftData)
{
  int i,j,repCnt;
  QColor c;
  double re,imag,tmp,tmp1;
  if((!showWaterfall) && (slowCPU))
    {
      if(displayCounter++<1) return;
      else displayCounter=0;
    }
  if((imWidth!=width()) || (imHeight!=height()))
    {
      if(imWidth!=width())
        {
          if(arMagWAvg!=NULL) delete [] arMagWAvg;
          arMagWAvg=new double[width()];
          for(i=0;i<width();i++)
            {
              arMagWAvg[i]=0;
            }
        }
      imWidth=width();
      imHeight=height();
      if(showWaterfall)
        {
          if(imagePtr==NULL)
            {
              imagePtr=new QImage( width(),height(),QImage::Format_RGB32);
              imagePtr->fill(Qt::black);
            }
          else
            {

              *imagePtr=imagePtr->scaled(QSize(imWidth,imHeight));
            }
        }
    }
  if(!showWaterfall)
    {
      for (i=binBegin,j=0;i<binEnd;i++,j++)
        {
          re=fftData[i]/fftLength;
          imag=fftData[fftLength-i]/fftLength;
          tmp=10*log10((re*re+imag*imag))-77.27;  // 0.5 Vtt is 0db
          if(arMagSAvg[j]<-100)
            {
              arMagSAvg[j]=-100;
            }
          if(arMagSAvg[j]<tmp)  arMagSAvg[j]=arMagSAvg[j]*(1-0.4)+0.4*tmp;
          else arMagSAvg[j]=arMagSAvg[j]*(1-avgVal)+avgVal*tmp;
          tmp=(fftMax-arMagSAvg[j])/range;
          if(tmp<0) tmp=0;
          if (tmp>1)tmp=1;
          int pos=(int)rint((double)(j*(imWidth-1))/(double)binDiff);
          fftArray->setPoint(j,pos,(imHeight-1)*tmp); // range 0 -> -1
        }
    }
  else
    {
      memmove(imagePtr->scanLine(1),imagePtr->scanLine(0),(imWidth*(imHeight-2))*sizeof(uint));
      uint *ptr=(uint *)imagePtr->scanLine(0);
      tmp1=0;
      repCnt=0;
      for (i=binBegin,j=0;i<binEnd;i++)
        {
          repCnt++;
          re=fftData[i]/fftLength;
          imag=fftData[fftLength-i]/fftLength;
          tmp1+=(re*re+imag*imag);
          if (((i-binBegin)*imWidth)/(binEnd-binBegin)>=j)
            {
              tmp1/=repCnt;
              repCnt=0;
              tmp=10*log10((tmp1))-77.27;  // 0.5 Vtt is 0db
              arMagWAvg[j]=arMagWAvg[j]*(1-avgVal)+avgVal*tmp;
              if( arMagWAvg[j]<-100)
                {
                  arMagWAvg[j]=-100;
                }
              tmp=1-(fftMax-arMagWAvg[j])/range;
              if(tmp<0) tmp=0;
              if (tmp>1)tmp=1;
              c.setHsv(240-tmp*60,255,tmp*255);
              ptr[j]=c.rgb();
              tmp1=0;
              j++;
            }
        }

    }
  update();
}


void fftDisplay::drawMarkers(QPainter *painter, int top, int bot)
{
  painter->drawLine((((marker1-FFTLOW)*imWidth)/FFTSPAN),top,(((marker1-FFTLOW)*imWidth)/FFTSPAN),bot);
  painter->drawLine((((marker2-FFTLOW)*imWidth)/FFTSPAN),top,(((marker2-FFTLOW)*imWidth)/FFTSPAN),bot);
  painter->drawLine((((marker3-FFTLOW)*imWidth)/FFTSPAN),top,(((marker3-FFTLOW)*imWidth)/FFTSPAN),bot);
}

QImage *fftDisplay::getImage()
{
  QImage *im = new QImage(width(),height()+10,QImage::Format_RGB32);
  QPainter p(im);
  QPen pn;
  if (!showWaterfall) {
    }
  else {
      if (imagePtr) {
          im->fill(Qt::black);
          p.drawImage(0,5,*imagePtr);
          pn.setColor(Qt::red);
          pn.setWidth(3);
          p.setPen(pn);
          drawMarkers(&p,0,4);
          drawMarkers(&p,height()+5,height()+9);
        }
    }
  return im;
}

void fftDisplay::paintEvent(QPaintEvent *p)
{
  QPen pn;
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  if(!showWaterfall)
    {
      pn.setColor(Qt::red);
      pn.setWidth(1);
      painter.setPen(pn);
      drawMarkers(&painter,0,imHeight);
      pn.setColor(Qt::green);
      painter.setPen(pn);
      painter.drawPolyline(*fftArray);
    }
  else
    {
      if(imagePtr)
        {
          scaledImage=imagePtr->scaled(QSize(width(),height()));
          painter.drawImage(0,0,scaledImage);
        }
    }
  QLabel::paintEvent(p);
}
