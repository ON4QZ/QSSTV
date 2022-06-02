#include "drmsegmentsview.h"
#include <QPainter>
#include <QDebug>
#include <QPaintEvent>

#include <math.h>


drmSegmentsView::drmSegmentsView(QWidget *parent) :QLabel(parent)

{
  maxBlocks=1;
  colFail=QColor(Qt::red);
  colOK=QColor(Qt::green);
  setFrameShape(QFrame::Box);
  setFrameShadow(QFrame::Sunken);
  setLineWidth(3);
  setScaledContents(true);
  blockListCount=-1;
}

drmSegmentsView::~drmSegmentsView()
{

}

void drmSegmentsView::paintEvent(QPaintEvent *e)
{
  int i;
  int blockX;
  float blockWidth;

  QRectF rct;
  QLabel::paintEvent(e);
//  if(blockListCount==blockList.count())
//    {
//      return;
//    }
//  blockListCount=blockList.count();
  QPainter painter(this);
  painter.setPen(QPen(colFail, 1, Qt::SolidLine));
  painter.setBrush(QBrush(colFail));
  painter.setRenderHint(QPainter::Antialiasing);

  rct=QRectF(contentsRect().left(),contentsRect().top() ,contentsRect().width()-4 ,contentsRect().height()-4 );
  blockWidth=(float)(contentsRect().width()-4)/maxBlocks;
  painter.drawRect(rct);
  painter.setBrush(QBrush(colOK));

  painter.setPen(QPen(colOK, 1, Qt::SolidLine));
  for(i=0;i<blockList.count();i++)
    {

      blockX=floor(blockList.at(i)*blockWidth)+contentsRect().left()+2;
      painter.drawRect(blockX,contentsRect().top(),ceil(blockWidth),contentsRect().height()-4);
    }
}

void drmSegmentsView::setColorFail(QColor color)
{
  colFail = color;

}

void drmSegmentsView::setColorOK(QColor color)
{
  colOK = color;
}

 void drmSegmentsView::setBlocks(QList<unsigned short> blkList)
 {
   blockList=blkList;
 }


