#ifndef BASEGRAPHICITEM_H
#define BASEGRAPHICITEM_H


#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

#include "gradientdialog.h"
#include <QGraphicsItem>

#define ARROWSIZE 12
#define RSIZE 10
#define HSIZE (RSIZE/2)

struct sitemParam
  {
 // must be set before returning parameters
    qreal zValue;
    int type;
    QPen pen;
    QBrush brush;
    QPointF position;

// are used dynamically, no need to setup
    QFont font;
    QString txt;
    int rotation;
    QImage im;
    double hShear;
    double vShear;
    sgradientParam gradient;
    QRectF rct;
    bool locked;
    QLineF line;
    bool modified;
    QColor fillColor;
    QMenu *menu;
};

enum ResizeCorners
{
  TOP_LEFT,
  TOP,
  TOP_RIGHT,
  RIGHT,
  BOTTOM_RIGHT,
  BOTTOM,
  BOTTOM_LEFT,
  LEFT,
  ROTATE
};




class graphItemBase: public QAbstractGraphicsShapeItem
{
public:
enum egraphType {BASE=QGraphicsItem::UserType+1,RECTANGLE,ELLIPSE,IMAGE,LINE,TEXT,REPLAY,SBORDER};
  graphItemBase(QMenu *cntxtMenu);
  ~graphItemBase();
  virtual void drawItem(QPainter *painter)=0;
  virtual QPainterPath shape() const;
  virtual QRectF boundingRect() const;
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
  void drawBorder(QPainter *painter);
  QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen) const;

  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
  void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
  bool mousePosOnHandles(QPointF pos);
  void load(QDataStream &str);
  void save(QDataStream &str);
  void setTransform ( int rot,double hs,double vs);
  QRectF rect() {return param.rct;}
  void setImage(QImage ima) {param.im=ima;}
  sitemParam getParam();
  void setParam(sitemParam sp);
  QString text() const { return param.txt;}

  virtual void setText(const QString &) { }
  virtual void setFont(QFont) {	}
  void setBrush(QColor c)
    {
      param.fillColor=c;
      QAbstractGraphicsShapeItem::setBrush(param.fillColor);
    }
  void setBrush(sgradientParam p,QRectF rct)
    {
      param.gradient=p;
      QAbstractGraphicsShapeItem::setBrush(buildGradient(param.gradient,rct));
    }
  void setLocked(bool b) {param.locked=b;}
  void setGradient(sgradientParam pm) { param.modified=true; param.gradient=pm;}
  virtual void setRect( const QRectF & rectangle )
    {
      param.rct=rectangle;
      param.modified=true;
    }
  virtual void setRect( qreal x, qreal y, qreal width, qreal height )
    {
      param.rct=QRectF(x,y,width,height);
      param.modified=true;
    }
  int type() { return param.type;}
  sitemParam *getParamPtr() {return &param;}
  bool markedForDeletion;


protected:

  QRectF m_ActualRect;
  QRectF m_CornerRect;
  bool selected;
  QVector<QRectF> m_ResizeHandles;
  //! Arrow line used as rotation handle
  QLineF m_RotateLine;
  //! Arrow head
  QPolygonF m_AngleHandle;
  double m_Angle;
  bool m_IsResizing;
  bool m_MousePressed;
  ResizeCorners m_ResizeCorner;

  sitemParam param;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    void setTransform ();
};

#define NUMITEMTYPES (graphItemBase::SBORDER-graphItemBase::BASE+1)
extern QString itemTypeStr[NUMITEMTYPES];

#endif // BASEGRAPHICITEM_H
