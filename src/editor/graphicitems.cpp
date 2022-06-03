#include "graphicitems.h"
#include <QPainter>

itemRectangle:: itemRectangle(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{
  param.type=RECTANGLE;
}

void itemRectangle::drawItem(QPainter *painter)
{
  QPen lpen(pen());
  lpen.setJoinStyle(Qt::MiterJoin);
  painter->setPen(lpen);
  painter->drawRect(param.rct);
}



itemEllipse:: itemEllipse(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{

  param.type=ELLIPSE;
}

void itemEllipse::drawItem(QPainter *painter)
{
  painter->drawEllipse(param.rct);
}


itemLine:: itemLine(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{
  param.type=LINE;
  param.rct.setHeight(2);
}

void itemLine::drawItem(QPainter *painter)
{ QPen lpen(pen());
  lpen.setCapStyle(Qt::FlatCap);
  painter->setPen(lpen);
  painter->drawLine(param.rct.x(),param.rct.y()+param.rct.height()/2,param.rct.x()+param.rct.width(),param.rct.y()+param.rct.height()/2);
}



itemImage:: itemImage(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{
  param.type=IMAGE;
}

void itemImage::drawItem(QPainter *painter)
{
  QImage tim;
  qreal pad=pen().widthF()/2;
  tim=param.im.scaled(param.rct.width(),param.rct.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

  painter->drawImage(param.rct.adjusted(2*pad,2*pad,-2*pad,-2*pad),tim);
  drawBorder(painter);
}


itemReplayImage:: itemReplayImage(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{
  param.type=REPLAY;
}

void itemReplayImage::drawItem(QPainter *painter)
{
  QImage tim;
  qreal pad=pen().widthF()/2;
  if (param.im.isNull())
    {
      QBrush b(Qt::black,Qt::Dense7Pattern);
      painter->setPen(pen());
      painter->setBrush(b);
      painter->drawRect(param.rct.adjusted(2*pad,2*pad,-2*pad,-2*pad));

    }
  else
    {
      tim=param.im.scaled(param.rct.width(),param.rct.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
      painter->drawImage(param.rct.adjusted(2*pad,2*pad,-2*pad,-2*pad),tim);
    }
  drawBorder(painter);
}



itemText:: itemText(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{
  param.type=TEXT;
  param.font.setStyleStrategy(QFont::ForceOutline);
}

void itemText::drawItem(QPainter *painter)
{
  int i;
  QPainterPath textPath;
  QPainterPath testPath;
  QPainterPath rectPath;
  qreal height=0;
  QStringList tlst=param.txt.split("\n");
  testPath.addText(0,0,param.font,"AQag");
  height=testPath.boundingRect().height()*1.2;
  for(i=0;i<tlst.count();i++)
    {
      textPath.addText(0, i*height,param.font, tlst.at(i));
    }
  painter->setPen(pen());
  if(param.gradient.type!=sgradientParam::NONE)
    {
      painter->setBrush(buildGradient(param.gradient,textPath.boundingRect()));


    }
  else
    {
      painter->setBrush(param.fillColor);
    }
    painter->drawPath(textPath);
//    painter->drawPath(rectPath);

}


QPainterPath itemText::shape() const
{
  int i;
  QPainterPath textPath;
  QPainterPath testPath;
  QPainterPath shapePath;

  textPath=QPainterPath();
  QStringList tlst=param.txt.split("\n");
  testPath.addText(0,0,param.font,"AQag");
  qreal height=testPath.boundingRect().height();
  for(i=0;i<tlst.count();i++)
    {
      textPath.addText(0, i*height*1.2,param.font, tlst.at(i));
    }
  shapePath.addRect(textPath.boundingRect());
  return qt_graphicsItem_shapeFromPath(shapePath,pen());
}


void itemText::setText(const QString &t)
{
  param.modified=true;
  param.txt=t;
  update();
}

void itemText::setFont(QFont f)
{
  param.modified=true;
  param.font=f;
  param.font.setStyleStrategy(QFont::ForceOutline);
  update();
}

itemBorder:: itemBorder(QMenu *cntxtMenu) : graphItemBase(cntxtMenu)
{
  param.type=SBORDER;
  setRect(0,0,100,100);
  setFlags(QGraphicsItem::QGraphicsItem::ItemIgnoresTransformations);
}

void itemBorder::drawItem(QPainter *painter)
{
  QPen pen(painter->pen());
  pen.setColor(Qt::black);
  pen.setWidth(1);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(pen);
  painter->drawRect(param.rct);
}

