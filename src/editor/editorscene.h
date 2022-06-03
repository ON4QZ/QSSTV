/***************************************************************************
 *   Copyright (C) 2000-2019 by Johan Maes                                 *
 *   on4qz@telenet.be                                                      *
 *   http://users.telenet.be/on4qz                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef EDITORSCENE_H
#define EDITORSCENE_H

#include <QGraphicsScene>
//#include <QGraphicsView>
//#include "graphics.h"
#include "graphicitems.h"
#include "supportfunctions.h"
#include "macroexpansion.h"

class QGraphicsSceneMouseEvent;

#define CHANGECOLOR
#define CHANGE





class editorScene : public QGraphicsScene
{
  Q_OBJECT

public:
  enum eImageType
  {
    NONE,				/*!< no image defined */
    FLATIMAGE, /*!< loaded image is a simple image (png,jpeg,...) */
    TEMPLATE    /*!< loaded image is a template file */
  };
  enum eMode { MOVE, INSERT};
  enum doChange {DNOCHANGE = 0, DFILLCOLOR = 1, DLINECOLOR=2, DGRADIENT=4,DTEXT = 8,DFONT=16,DPEN=32,DTRANSFORM=64};
  Q_DECLARE_FLAGS(changeFlags, doChange);
  editorScene(QGraphicsView *parent=0);
  ~editorScene();
  QColor fillColor;
  QColor lineColor;
  QGradient gradient;
  QColor gradientColor;
  QFont  font;
  QString text;
  QString fl;
  double penWidth;
  void apply(changeFlags cf);
  void clearAll();
  QRectF border;
  int rotate;
  qreal hShear;
  qreal vShear;
  bool load(QFile &f);
  bool save(QFile &f,bool templ);
  void setImage(QImage *im);
  eMode mode;
  eImageType getImageType(){return imageType;}
  QImage *renderImage(int w, int h);
  macroExpansion mexp;
  void overlay(QImage *ima);
  void addBorder(int w,int h);
  QImage *getImagePtr() {return localImage;}
  void addConversion(QChar tag,QString value,bool clear=false);

  // bool event(QEvent *);

public slots:
  void setMode(eMode m);
  void setItemType(graphItemBase::egraphType tp);
  //    void editorLostFocus(DiagramTextItem *item);
  void slotCopy();
  void slotPaste();
  void slotExpand();

  void slotDeleteItem();
  void slotLock();
  void slotUnlock();
  void slotBringToFront();
  void slotSendToBack();
  void slotSendBackward();
  void slotSendForward();
  void slotChangeText();
#ifndef QT_NO_DEBUG
  void slotProperties();
  void dumpItems();
#endif


signals:
  //	void itemInserted(graphItemBase *itm);
  //	void textInserted(graphItemBase *itm);
  void changeSize(int,int);
  void itemSelected(graphItemBase *itm);
  void colorSelected( const QPointF &p);

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);


private:
  graphItemBase *copyItem;
  graphItemBase::egraphType itemType;
  bool leftButtonDown;

  bool pasted;
  qreal zMax;
  QPointF startPoint;
  // Context menus
  QMenu *contextMenu;
  QMenu *arrange;

  void optimizeDepth();
  void itemSetup(graphItemBase *item);
  void makeCopy(graphItemBase *it);
  eImageType imageType;
  QImage *localImage;
  void flattenImage(int w,int h);
  void convertText();
  //  QString textConversion(QString str);
  //  QString convertedText;
  graphItemBase *borderItemPtr;
  void convertReplayImage();

};
Q_DECLARE_OPERATORS_FOR_FLAGS(editorScene::changeFlags)

#endif
