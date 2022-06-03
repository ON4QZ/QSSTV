#include "markerwidget.h"
#include "fftdisplay.h"
#include "QDebug"

markerWidget::markerWidget(QWidget *parent) :  QLabel(parent)
{
}


void markerWidget::paintEvent(QPaintEvent *p)
{
  QPen pn;
  QPainter painter(this);
  pn.setColor(Qt::red);
  pn.setWidth(2);
  painter.setPen(pn);

  if ((marker1>FFTLOW)&&(marker1<(FFTLOW+FFTSPAN)))
    {
      painter.drawLine((((marker1-FFTLOW)*width())/FFTSPAN),0,(((marker1-FFTLOW)*width())/FFTSPAN),height());
    }
  if ((marker2>FFTLOW)&&(marker2<(FFTLOW+FFTSPAN))) painter.drawLine((((marker2-FFTLOW)*width())/FFTSPAN),0,(((marker2-FFTLOW)*width())/FFTSPAN),height());
  if ((marker3>FFTLOW)&&(marker3<(FFTLOW+FFTSPAN))) painter.drawLine((((marker3-FFTLOW)*width())/FFTSPAN),0,(((marker3-FFTLOW)*width())/FFTSPAN),height());
  QLabel::paintEvent(p);
}
