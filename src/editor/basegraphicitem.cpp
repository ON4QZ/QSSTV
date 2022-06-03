#include "basegraphicitem.h"

#include "basegraphicitem.h"
#include "editorscene.h"

#include <QPainter>
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QString>
#include <QDebug>
#include <math.h>



QString itemTypeStr[NUMITEMTYPES]=
{
  "Base",
  "Rectangle",
  "Ellipse",
  "Image",
  "Line",
  "Text",
  "Replay",
  "Border"
};

const qreal penWidthZero = qreal(0.00000001);

QPainterPath graphItemBase::qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen) const
{
  // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
  // if we pass a value of 0.0 to QPainterPathStroker::setWidth()


  if (path == QPainterPath()) return path;
  QPainterPathStroker ps;
  ps.setCapStyle(pen.capStyle());
  if (pen.widthF() <= 0.0) ps.setWidth(penWidthZero);
  else  ps.setWidth(pen.widthF());
  ps.setJoinStyle(pen.joinStyle());
  ps.setMiterLimit(pen.miterLimit());
  QPainterPath p = ps.createStroke(path);
  p.addPath(path);
  return p;
}


graphItemBase::graphItemBase(QMenu *cntxtMenu)
{
  param.rct = QRectF(0, 0, 100, 100);
  m_ResizeHandles.fill(QRect(0, 0, 0, 0), 8); //initially empty handles
  m_MousePressed = false;
  m_IsResizing = false;
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsSelectable);
  param.locked=false;
  param.modified=true;
  param.menu=cntxtMenu;
  param.pen.setJoinStyle(Qt::MiterJoin);
  setAcceptHoverEvents(true);
  selected=false;
  markedForDeletion=false;

}

graphItemBase::~graphItemBase()
{

  prepareGeometryChange();
}

QPainterPath graphItemBase::shape() const
{
  QPainterPath path;
  path.addRect(param.rct.adjusted(-2*HSIZE, -2*HSIZE, 2*HSIZE, 2*HSIZE));
  return qt_graphicsItem_shapeFromPath(path,pen());
}

QRectF graphItemBase::boundingRect() const
{
  return shape().boundingRect();
}


void graphItemBase::load(QDataStream &str)
{
  //	str << type(); this item is already read by the loader
  QTransform f;
  QPointF p;
  QRectF r;
  QColor c;
  QPen pn;
  QBrush br;
  QString t;
  QFont fnt;
  qreal z;
  str >> param.rotation;
  str >> param.hShear;
  str >> param.vShear;
  str >> z;
  setZValue(z);
  param.zValue=z;
  str >> p;
  setPos(p);
  param.position=p;
  str >> r;
  setRect(r);
  str >> c;
  param.fillColor=c;
  setBrush(c);
  str >> pn;
  setPen(pn);
  str >> br;
  QAbstractGraphicsShapeItem::setBrush(br);
  str >> param.locked;
  str >>param.im;
  str >>t;
  setText(t);
  str >>fnt;
  setFont(fnt);
  str >> param.line;
  param.gradient.load(str);
  setTransform ();
  getParam();
}

void graphItemBase::save(QDataStream &str)
{
  str << type();
  str << param.rotation;
  str << param.hShear;
  str << param.vShear;
  str	<< zValue();
  str << pos();
  str << rect();
  str << param.fillColor;
  str << pen();
  str << brush();
  str << param.locked;
  str << param.im;
  str << param.txt;
  str << param.font;
  str << param.line;
  param.gradient.save(str);
}

void graphItemBase::setTransform(int rot, double hs, double vs)
{
  param.rotation=rot;
  param.hShear=hs;
  param.vShear=vs;
  setTransform ();
}

void graphItemBase::setTransform ()
{
  QTransform tx;
  tx.translate(rect().x()+rect().width()/2,rect().y()+rect().height()/2);
  tx.shear(param.hShear,param.vShear);
  tx.rotate(param.rotation);
  tx.translate(-rect().x()-rect().width()/2,-rect().y()-rect().height()/2);
  QGraphicsItem::setTransform(tx,false);
  update();
}

sitemParam graphItemBase::getParam()
{
  param.zValue=zValue();
  param.type=type();
  param.pen=pen();
  param.brush=brush();
  param.position=pos();
  return param;
}

void graphItemBase::setParam(sitemParam sp)
{
  setPen(sp.pen);
  setBrush(sp.fillColor);
  setFont(sp.font);
  // are used dynamically, no need to setup
  param.txt=sp.txt;
  param.rotation=sp.rotation;
  param.hShear=sp.hShear;
  param.vShear=sp.vShear;
  setTransform (param.rotation,param.hShear,param.vShear);
  param.im=sp.im;
  param.gradient=sp.gradient;
  param.rct=sp.rct;
  param.locked=sp.locked;
  param.line=sp.line;
  param.modified=true;
  param.fillColor=sp.fillColor;
  param.menu=sp.menu;
}

void graphItemBase::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  Q_UNUSED(widget)
  QPointF center;
  QRectF br;
  double scale;
  if (option->state & QStyle::State_Selected)
    {
      selected=true;
    }
  else
    {
      selected=false;
    }
  if(!scene()) return;
  if(scene()->views().isEmpty())
    {
      scale=1;
    }
  else
    {
      scale = scene()->views().at(0)->transform().m11(); //get current scale factor
    }


  float rectSize = RSIZE/ scale; //this is to maintain same size for resize handle rects
  QRectF handles(0, 0, rectSize, rectSize);
  br=boundingRect();
  if(selected)
    {

      painter->fillRect(br, QBrush(QColor(128, 128, 255, 128)));
      if(param.type!=LINE)
        {

          //resize handles
          handles.moveTopLeft(br.topLeft());  //TopLeft
          m_ResizeHandles.replace(0, handles);

          handles.moveTopRight(br.topRight()); //TopRight
          m_ResizeHandles.replace(2, handles);

          handles.moveBottomRight(br.bottomRight());  //BottomRight
          m_ResizeHandles.replace(4, handles);

          handles.moveBottomLeft(br.bottomLeft()); //BottomLeft
          m_ResizeHandles.replace(6, handles);

          center= QPointF(br.center().x(), br.top()+HSIZE); //Top
          handles.moveCenter(center);
          m_ResizeHandles.replace(1, handles);

          center = QPointF(br.center().x(), br.bottom()-HSIZE);  //Bottom
          handles.moveCenter(center);
          m_ResizeHandles.replace(5, handles);
        }

      center = QPointF(br.right()-HSIZE, br.center().y()); //Right
      handles.moveCenter(center);
      m_ResizeHandles.replace(3, handles);

      center = QPointF(br.left()+HSIZE, br.center().y());  //Left
      handles.moveCenter(center);
      m_ResizeHandles.replace(7, handles);

      //arrow line
      float size = br.width();
      m_RotateLine.setP1(m_ResizeHandles.at(7).center());
      m_RotateLine.setP2(QPointF(m_ResizeHandles.at(7).center().x() + (size / 4), m_ResizeHandles.at(7).center().y()));

      //angle handle
      qreal arrowSize = ARROWSIZE / scale;
      QPointF point = m_RotateLine.p2();
      double angle = ::acos(m_RotateLine.dx() / m_RotateLine.length());
      if (m_RotateLine.dy() >= 0)
        angle = (2.0 * M_PI) - angle;
      QPointF destArrowP1 = point + QPointF(sin(angle - M_PI / 3.0) * arrowSize, cos(angle - M_PI / 3.0) * arrowSize);
      QPointF destArrowP2 = point + QPointF(sin(angle - M_PI + M_PI / 3.0) * arrowSize , cos(angle - M_PI + M_PI / 3.0) * arrowSize);
      //  qreal points[] = { point.x(), point.y(), destArrowP1.x(), destArrowP1.y(), destArrowP2.x(), destArrowP2.y() };
      m_AngleHandle = QPolygonF() << m_RotateLine.p2() << destArrowP1 << destArrowP2;
    }
  if(param.gradient.type!=sgradientParam::NONE && (param.type==ELLIPSE|| param.type==RECTANGLE))
    {
      setBrush(param.gradient,rect());
    }
  else
    {
      setBrush(param.fillColor);
    }
  painter->setBrush(brush());
  painter->setPen(pen());
  drawItem(painter);

  QPen pens;
  pens.setCosmetic(true); //to maintain same width of pen across zoom levels

  painter->setPen(pens);

  if(selected)
    {
      //draw arrow handle
      QPen arrowPen;
      arrowPen.setCosmetic(true);
      arrowPen.setColor(Qt::yellow);
      painter->setBrush(Qt::black);
      painter->setPen(arrowPen);
      painter->drawLine(m_RotateLine);
      painter->drawPolygon(m_AngleHandle);
      if(param.type!=TEXT)
        {
          //draw resize handles
          pens.setColor(QColor(255, 255, 255));
          painter->setBrush(Qt::black);
          painter->setPen(pens);
          painter->drawRects(m_ResizeHandles);
        }
    }
}

void graphItemBase::drawBorder(QPainter *painter)
{
  QPen lpen(pen());
  qreal pad=lpen.widthF()/2;
  painter->setBrush(Qt::NoBrush);
  lpen.setJoinStyle(Qt::MiterJoin);
  painter->setPen(lpen);
  painter->drawRect(param.rct.adjusted(pad,pad,-pad,-pad));
}


void graphItemBase::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

  m_MousePressed = true;
  m_IsResizing = mousePosOnHandles(event->scenePos()); //to check event on corners or not
  if (m_IsResizing)
    {
      m_ActualRect = param.rct;
    }
  QGraphicsItem::mousePressEvent(event);
}

void graphItemBase::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if(param.locked) return;

  QRectF m_BoundingRect(boundingRect());

  bool containsAll=false;
  bool containsNoText=false;
  bool containsNoLineNoText=false;
  bool changed=false;

  QRectF escene=scene()->sceneRect().adjusted(-2*HSIZE,-2*HSIZE,2*HSIZE,2*HSIZE);
  if (escene.contains(event->scenePos()))
    {
      containsAll=true;
      //      containsNoLine=(param.type!=LINE);
      containsNoLineNoText=((param.type!=LINE) && (param.type!=TEXT));
      containsNoText=(param.type!=TEXT);
    }
  if (m_IsResizing)
    {
      QPointF ptMouseMoveInItemsCoord = mapFromScene(event->scenePos());

      if(m_ResizeCorner==TOP_LEFT && containsNoLineNoText)
        {
          changed=true;
          m_BoundingRect.setTopLeft(ptMouseMoveInItemsCoord);
        }


      else if(m_ResizeCorner==TOP_RIGHT && containsNoLineNoText)
        {
          changed=true;
          m_BoundingRect.setTopRight(ptMouseMoveInItemsCoord);
        }


      else if(m_ResizeCorner==BOTTOM_LEFT && containsNoLineNoText)
        {
          changed=true;
          m_BoundingRect.setBottomLeft(ptMouseMoveInItemsCoord);
        }


      else if(m_ResizeCorner==BOTTOM_RIGHT && containsNoLineNoText)
        {
          changed=true;
          m_BoundingRect.setBottomRight(ptMouseMoveInItemsCoord);
        }

      else if(m_ResizeCorner==TOP && containsNoLineNoText)
        {
          changed=true;
          m_BoundingRect.setTop(ptMouseMoveInItemsCoord.y());

        }

      else if(m_ResizeCorner==BOTTOM && containsNoLineNoText)
        {
          changed=true;
          m_BoundingRect.setBottom(ptMouseMoveInItemsCoord.y());

        }

      else if(m_ResizeCorner==LEFT && containsNoText)
        {
          changed=true;
          m_BoundingRect.setLeft(ptMouseMoveInItemsCoord.x());
        }

      else if(m_ResizeCorner==RIGHT && containsNoText)
        {
          changed=true;
          m_BoundingRect.setRight(ptMouseMoveInItemsCoord.x());
        }

      else if(m_ResizeCorner==ROTATE && containsAll)
        {

          changed=true;
          QLineF line(m_BoundingRect.center(), ptMouseMoveInItemsCoord);
          double rotations = line.angleTo(QLineF(0, 0, 1, 0));
          if (line.dy() <= 0)
            {
              rotations = 180.0 - rotations;
            }
          else
            {
              rotations = rotations - 180.0;
            }
          m_Angle = rotations;
          m_BoundingRect = m_BoundingRect.normalized();
          setTransformOriginPoint(m_BoundingRect.center());
          setRotation(rotation() + m_Angle);
          param.rotation=round(rotation());
        }
      if(changed)
        {
          if(m_BoundingRect.width()<4*HSIZE) m_BoundingRect.setWidth(4*HSIZE);
          if(m_BoundingRect.height()<4*HSIZE) m_BoundingRect.setHeight(4*HSIZE);
          param.rct = m_BoundingRect.adjusted(2*HSIZE,2*HSIZE, -2*HSIZE, -2*HSIZE);
          qreal penw=pen().widthF()/2;
          param.rct = param.rct.adjusted(penw,penw,-penw,-penw);
          prepareGeometryChange();
          update();
        }

    }
  if(!changed)
    {
      prepareGeometryChange();
      update();
      QGraphicsItem::mouseMoveEvent(event);
    }
}

void graphItemBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QRectF m_BoundingRect(param.rct.adjusted(-2*HSIZE, -2*HSIZE, 2*HSIZE, 2*HSIZE));
  m_MousePressed = false;
  m_IsResizing = false;
  if (m_ActualRect != m_BoundingRect)
    { // Rotating won't trigger this, only resizing.
      QPointF oldScenePos = scenePos();
      setTransformOriginPoint(m_BoundingRect.center());
      QPointF newScenePos = scenePos();
      QPointF oldPos = pos();
      setPos(oldPos.x() + (oldScenePos.x() - newScenePos.x()), oldPos.y() + (oldScenePos.y() - newScenePos.y()));
    }
  prepareGeometryChange();
  update();
  QGraphicsItem::mouseReleaseEvent(event);
}

void graphItemBase::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
}

void graphItemBase::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{

}

void graphItemBase::hoverMoveEvent(QGraphicsSceneHoverEvent *)
{
  if(param.type==SBORDER) return;
  if(param.locked)
    {
      setCursor(Qt::ForbiddenCursor);
    }
  else
    {
      setCursor(Qt::ArrowCursor);
    }
}

void graphItemBase::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{

}

void graphItemBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  if(param.type==SBORDER) return;
  setSelected(true);
  param.menu->exec(event->screenPos());
}


bool graphItemBase::mousePosOnHandles(QPointF pos)
{
  bool resizable = false;
  int rem4Index = 8;

  if(param.type!=LINE)
    {
      if (mapToScene(m_ResizeHandles[(0 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = TOP_LEFT;
          resizable = true;
        }
      else if (mapToScene(m_ResizeHandles[(1 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = TOP;
          resizable = true;
        }
      else if (mapToScene(m_ResizeHandles[(2 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = TOP_RIGHT;
          resizable = true;
        }

      else if (mapToScene(m_ResizeHandles[(4 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = BOTTOM_RIGHT;
          resizable = true;
        }
      else if (mapToScene(m_ResizeHandles[(5 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = BOTTOM;
          resizable = true;
        }
      else if (mapToScene(m_ResizeHandles[(6 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = BOTTOM_LEFT;
          resizable = true;
        }
    }
  if(param.type!=TEXT)
    {
      if (mapToScene(m_ResizeHandles[(3 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = RIGHT;
          resizable = true;
        }
      else if (mapToScene(m_ResizeHandles[(7 + rem4Index) % 8]).containsPoint(pos, Qt::WindingFill))
        {
          m_ResizeCorner = LEFT;
          resizable = true;
        }
    }
  if (mapToScene(m_AngleHandle).containsPoint(pos, Qt::WindingFill))
    {
      m_ResizeCorner = ROTATE;
      resizable = true;
    }
  return resizable;
}
