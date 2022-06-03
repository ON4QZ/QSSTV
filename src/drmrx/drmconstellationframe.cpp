#include "drmconstellationframe.h"
#include "ui_drmconstellationframe.h"
#include <QPainter>
#include "drm.h"
#include "math.h"
#include "appdefs.h"


extern float MSC_cells_sequence[2 * 2959];
extern int lMSC;
extern bool MSCAvailable;

extern int lFAC;
extern float FAC_cells_sequence[200];
extern bool FACAvailable;

#define CSTRANGE 1.5
#define CSTSPAN 3.

drmConstellationFrame::drmConstellationFrame(QWidget *parent) :
  QFrame(parent),
  ui(new Ui::drmConstellationFrame)
{
  ui->setupUi(this);
  lmsc=0;
}

drmConstellationFrame::~drmConstellationFrame()
{
  delete ui;
}

void drmConstellationFrame::paintEvent (QPaintEvent *e)
{
    int i,posx,posy;
    QPainter qpainter (this);
   // qpainter.drawRect (contentsRect());
    qpainter.setPen (QPen (Qt::blue, 2));
    qpainter.drawLine (contentsRect().x()+contentsRect().width()/2, contentsRect().y(), contentsRect().x()+contentsRect().width()/2,contentsRect().y()+contentsRect().height());
    qpainter.drawLine (contentsRect().x(), contentsRect().y()+contentsRect().height()/2, contentsRect().x()+contentsRect().width(),contentsRect().y()+contentsRect().height()/2);
    for(i=0;i<lmsc/2;i++)
    {
        posx=rint(((ConstellationArray[2*i]+CSTRANGE)/CSTSPAN)*(float)contentsRect().width());
        posy=contentsRect().height()-rint(((ConstellationArray[2*i+1]+CSTRANGE)/CSTSPAN)*(float)contentsRect().height());
        qpainter.drawEllipse(posx,posy,2,2);
    }
  QFrame::paintEvent(e);
 }

void drmConstellationFrame::setConstellation(econstellation constellation)
{
  if (MSCAvailable && constellation==MSC)
    {
      lmsc=lMSC;
      for (int i=0; i < lMSC; i++)
        {
          ConstellationArray[i]=MSC_cells_sequence[i];
        }
      update();
      MSCAvailable=false;
    }
  if (FACAvailable  && constellation==FAC)
    {
      lmsc=lFAC;
      for (int i=0; i < lmsc; i++)
        {
          ConstellationArray[i]=FAC_cells_sequence[i];
        }
      update();
      FACAvailable=false;
    }
}

void drmConstellationFrame::clearConstellation()
{
  lmsc=0;
  update();
}
