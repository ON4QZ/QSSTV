#include "drmpsdframe.h"
#include "ui_drmpsdframe.h"
#include <QPainter>
#include "drm.h"
#include "math.h"

#define PSDRANGE 70.
#define PSDLOW  0.

drmPSDFrame::drmPSDFrame(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::drmPSDFrame)
{
  ui->setupUi(this);
}

drmPSDFrame::~drmPSDFrame()
{
  delete ui;
}

void drmPSDFrame::paintEvent (QPaintEvent *e)
{
    int i,x1,x2,y1,y2;
    float y;
    QPainter qpainter (this);

    //qpainter.drawRect (contentsRect());
    qpainter.setPen (QPen (Qt::blue, 1));
    qpainter.drawLine (contentsRect().x()+contentsRect().width()/2, contentsRect().y(), contentsRect().x()+contentsRect().width()/2,contentsRect().y()+contentsRect().height());
    qpainter.drawLine (contentsRect().x(), contentsRect().y()+contentsRect().height()/2, contentsRect().x()+contentsRect().width(),contentsRect().y()+contentsRect().height()/2);
    // draw PSD Info
    x1=0;
    y1=contentsRect().height();
    for(i=0;i<PSDSPAN/4;i++)
    {
        y=psd[i]-PSDLOW;
        if (y>PSDRANGE) y=PSDRANGE;
        y2=contentsRect().height()-rint((y/PSDRANGE) *(float)contentsRect().height());
        x2=(i*contentsRect().width())/(PSDSPAN/4);
        qpainter.drawLine(x1,y1,x2,y2);
        x1=x2;y1=y2;
    }
       qpainter.setPen (QPen (Qt::red, 1));
       x1=0;
       y1=contentsRect().height();
       for(i=0;i<PSDSPAN/4;i++)
       {
           y=cpsd[i]-PSDLOW;
           if (y>PSDRANGE) y=PSDRANGE;
           y2=contentsRect().height()-rint((y/PSDRANGE) *(float)contentsRect().height());
           x2=(i*contentsRect().width())/(PSDSPAN/4);
           qpainter.drawLine(x1,y1,x2,y2);
           x1=x2;y1=y2;
       }

  QFrame::paintEvent(e);
 }

void drmPSDFrame::setPSD()
{
   for(int i=0;i<PSDSPAN/4;i++)
    {
        psdArray[i]=psd[i];
    }
   for(int i=0;i<PSDSPAN/4;i++)
    {
        psdCArray[i]=cpsd[i];
    }
   update();
}
