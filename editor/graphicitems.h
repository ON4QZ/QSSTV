#ifndef GRAPHICITEMS_H
#define GRAPHICITEMS_H


#include "basegraphicitem.h"


class itemRectangle : public graphItemBase
{
public:
  itemRectangle(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  RECTANGLE;}

};

class itemEllipse : public graphItemBase
{
public:
  itemEllipse(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  ELLIPSE;}
};


class itemLine : public graphItemBase
{
public:
  itemLine(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  LINE;}
//  QPainterPath shape() const;
};

class itemImage : public graphItemBase
{
public:
  itemImage(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  IMAGE;}
};

class itemText : public graphItemBase
{
public:
  itemText(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  TEXT;}
  QPainterPath shape() const;
  void setText(const QString &t);
  void setFont(QFont f);
  QRectF boundingRct;

};

class itemReplayImage : public graphItemBase
{
public:
  itemReplayImage(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  REPLAY;}
};

class itemBorder : public graphItemBase
{
public:
  itemBorder(QMenu *cntxtMenu);
  void drawItem(QPainter *painter);
  int type() const {return  SBORDER;}
};



#endif // GRAPHICITEMS_H
