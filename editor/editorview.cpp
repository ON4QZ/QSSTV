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

#include "editorview.h"
#include "appglobal.h"
#include "dirdialog.h"
#include "graphicitems.h"
#include "gradientdialog.h"
#include "canvassizeform.h"
#include "ui_textform.h"

#include <QFontInfo>


#define TBFILL 0
#define TBLINE 1
#define TBGRAD 2


struct sCanvasSize
{
  const QString s;
  int width;
  int height;
};

#define BORDER 4
/** editorview */

editorView::editorView(QWidget *parent):QWidget(parent), Ui::editorForm()
{
  setupUi(this);
  scene=new editorScene(canvas);
  canvas->setScene(scene);
  scene->setMode(editorScene::MOVE);
  scene->setItemType(graphItemBase::BASE);


  fontComboBox->setFontFilters(QFontComboBox::ProportionalFonts);
  fontSizeSpinBox->setRange(6, 180);
  penWidthSpinBox->setRange(0,99);




  readSettings();

  connect(imageSizePushButton,SIGNAL(clicked()),SLOT(slotChangeCanvasSize()));
  connect(scene,SIGNAL(itemSelected(graphItemBase*)),SLOT(slotItemSelected(graphItemBase*)));
  connect(rectanglePushButton,SIGNAL(clicked()),SLOT(slotRectangle()));
  connect(circlePushButton,SIGNAL(clicked()),SLOT(slotCircle()));
  connect(replayPushButton,SIGNAL(clicked()),SLOT(slotReplay()));
  connect(imagePushButton,SIGNAL(clicked()),SLOT(slotImage()));
  connect(linePushButton,SIGNAL(clicked()),SLOT(slotLine()));
  connect(textPushButton,SIGNAL(clicked()),SLOT(slotText()));


  hshearLCD->display( "0.00" );
  vshearLCD->display( "0.00" );
  connect(hshearSlider, SIGNAL(valueChanged(int)),SLOT(slotShearChanged(int)) );
  connect(vshearSlider, SIGNAL(valueChanged(int)),SLOT(slotShearChanged(int)) );


  connect(fontComboBox,SIGNAL(currentFontChanged(const QFont &)),SLOT(slotFontChanged(const QFont &)));
  connect(fontSizeSpinBox,SIGNAL( valueChanged (int)),SLOT(slotFontSizeChanged(int)));
  connect(penWidthSpinBox,SIGNAL( valueChanged (double)),SLOT(slotPenWidthChanged(double)));
  connect(boldButton,SIGNAL( clicked(bool)),SLOT(slotBold(bool)));
  connect(italicButton,SIGNAL( clicked(bool)),SLOT(slotItalic(bool)));
  connect(underlineButton,SIGNAL( clicked(bool)),SLOT(slotUnderline(bool)));


  fillToolButton->setMenu(createColorMenu(SLOT(slotColorDialog()),TBFILL,"Select Color"));
  fillToolButton->setIcon(createColorToolButtonIcon(":/icons/colorfill.png", scene->fillColor));
  connect(fillToolButton, SIGNAL(clicked()),this, SLOT(slotButtonTriggered()));

  lineToolButton->setMenu(createColorMenu(SLOT(slotColorDialog()),TBLINE,"Select Color"));
  lineToolButton->setIcon(createColorToolButtonIcon(":/icons/colorline.png", scene->lineColor));
  connect(lineToolButton, SIGNAL(clicked()),this, SLOT(slotButtonTriggered()));

  gradientToolButton->setMenu(createColorMenu(SLOT(slotGradientDialog()),TBGRAD,"Select Gradient"));
  gradientToolButton->setIcon(createColorToolButtonIcon(":/icons/gradient.png", scene->gradientColor));
  connect(gradientToolButton, SIGNAL(clicked()),this, SLOT(slotButtonTriggered()));

  // setup the defaults
  setTransform();
  slotFontChanged(fontComboBox->currentFont());
  slotPenWidthChanged(penWidth);
  changeCanvasSize();
#ifdef QT_NO_DEBUG
  dumpPushButton->hide();
#else
  connect(dumpPushButton,SIGNAL(clicked()),SLOT(slotDump()));
#endif

  modified=false;
}


editorView::~editorView()
{
  QMenu *menuPtr;
  menuPtr=fillToolButton->menu();
  if(menuPtr) delete menuPtr;
  menuPtr=lineToolButton->menu();
  if(menuPtr) delete menuPtr;
  menuPtr=gradientToolButton->menu();
  if(menuPtr) delete menuPtr;
  writeSettings();
}

/*! 
  reads the settings (saved images for tx,rx,templates)
*/

void editorView::readSettings()
{
  QSettings qSettings;

  qSettings.beginGroup ("Editor");
  canvasWidth=qSettings.value("canvasWidth", 800 ).toInt();
  canvasHeight=qSettings.value("canvasHeight", 600 ).toInt();
  fontFamily=qSettings.value("fontFamily", qApp->font().family()).toString();
  bold=qSettings.value("bold", false ).toBool();
  underline=qSettings.value("currentUnderline", false ).toBool();
  italic=qSettings.value("italic", false ).toBool();
  pointSize=qSettings.value("pointSize", 24).toInt();
  penWidth=qSettings.value("penwidth", 1).toDouble();
  fillColor = qSettings.value("fillcolor", QColor(Qt::white)).value<QColor>();
  lineColor = qSettings.value("linecolor", QColor(Qt::black )).value<QColor>();
  gradientColor=qSettings.value("gradientcolor",QColor( Qt::red )).value<QColor>();
  qSettings.endGroup();
  setParams();
}

/*! 
  writes the settings (saved images for tx,rx,templates)
*/
void editorView::writeSettings()
{

  QSettings qSettings;
  getParams();
  qSettings.beginGroup ("Editor" );
  qSettings.setValue ("canvasWidth",canvasWidth);
  qSettings.setValue ("canvasHeight",canvasHeight);
  qSettings.setValue ("fontFamily", fontFamily);
  qSettings.setValue ("bold", bold);
  qSettings.setValue ("underline", underline);
  qSettings.setValue ("italic", italic);
  qSettings.setValue ("pointSize",pointSize);
  qSettings.setValue ("penwidth",penWidth);
  qSettings.setValue ("fillcolor", fillColor);
  qSettings.setValue ("linecolor", lineColor);
  qSettings.setValue ("gradientcolor", gradientColor);
  qSettings.endGroup();
}

void editorView::getParams()
{
  getValue(bold,boldButton);
  getValue(underline,underlineButton);
  getValue(italic,italicButton);
  getValue(pointSize,fontSizeSpinBox);
  getValue(penWidth,penWidthSpinBox);
  fontFamily=fontComboBox->currentFont().family();

  fillColor=scene->fillColor;
  lineColor=scene->lineColor;
  gradientColor=scene->gradientColor;
}

void editorView::setParams()
{
  QFont fnt;
  fontComboBox->setCurrentIndex(fontComboBox->findText(fontFamily));
  setValue(bold,boldButton);
  setValue(underline,underlineButton);
  setValue(italic,italicButton);
  setValue(pointSize,fontSizeSpinBox);
  setValue(penWidth,penWidthSpinBox);
  fnt.setFamily(fontFamily);
  fnt.setBold(bold);
  fnt.setUnderline(underline);
  fnt.setItalic(italic);
  fnt.setPointSize(pointSize);
  scene->setFont(fnt);
  scene->fillColor = fillColor;
  scene->lineColor = lineColor;
  scene->gradientColor = gradientColor;
}



void editorView::slotRectangle()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(graphItemBase::RECTANGLE);
  modified=true;
}


void editorView::slotCircle()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(graphItemBase::ELLIPSE);
  modified=true;
}

void editorView::slotText()
{
  QDialog d(this);
  Ui::textForm t;
  t.setupUi(&d);
  t.plainTextEdit->setPlainText(txt);
  if(d.exec()==QDialog::Accepted)
    {
      scene->setMode(editorScene::INSERT);
      scene->setItemType(graphItemBase::TEXT);
      scene->text=t.plainTextEdit->toPlainText();
      txt=t.plainTextEdit->toPlainText();
      scene->apply(editorScene::DTEXT);
    }
  modified=true;
}

void editorView::slotLine()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(graphItemBase::LINE);
  modified=true;
}


void editorView::slotImage()
{
  QString fileName;
  dirDialog dd((QWidget *)this,"editor");
  scene->fl=dd.openFileName(QString());
  scene->setMode(editorScene::INSERT);
  scene->setItemType(graphItemBase::IMAGE);
  modified=true;
}

void editorView::slotReplay()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(graphItemBase::REPLAY);
  modified=true;
}

void editorView::changeCanvasSize()
{
  hshearSlider->setValue(0);
  vshearSlider->setValue(0);
  setTransform();
  scene->addBorder(canvasWidth,canvasHeight);
  canvas->setSceneRect(0,0,canvasWidth,canvasHeight);
  modified=true;
}


void editorView::slotChangeCanvasSize()
{
  QRect r;
  canvasSizeForm csize;
  csize.setSize(canvasWidth,canvasHeight);
  if(csize.exec()==QDialog::Accepted)
    {
      r=csize.getSize();
      canvasWidth=r.width();
      canvasHeight=r.height();
      changeCanvasSize();
    }
}

void editorView::slotFontChanged(const QFont &f)
{
  scene->font=f;
  scene->font.setPointSize(fontSizeSpinBox->value());
  scene->font.setBold(boldButton->isChecked());
  scene->font.setItalic(italicButton->isChecked());
  scene->font.setUnderline(underlineButton->isChecked());
  scene->apply(editorScene::DFONT);
  modified=true;
}

void editorView::slotFontSizeChanged(int sz)
{
  scene->font.setPointSize(sz);
  scene->apply(editorScene::DFONT);
  modified=true;
}

void editorView::slotPenWidthChanged(double pw)
{
  scene->penWidth=pw;
  scene->apply(editorScene::DPEN);
  modified=true;
}
void editorView::slotBold(bool b)
{
  scene->font.setBold(b);
  scene->apply(editorScene::DFONT);
  modified=true;
}
void editorView::slotItalic(bool b)
{
  scene->font.setItalic(b);
  scene->apply(editorScene::DFONT);
  modified=true;
}
void editorView::slotUnderline(bool b)
{
  scene->font.setUnderline(b);
  scene->apply(editorScene::DFONT);
  modified=true;
}

void editorView::setImage(QImage *)
{

  scene->setMode(editorScene::INSERT);
  scene->setItemType(graphItemBase::IMAGE);
  modified=true;
}


void editorView::slotClearAll()
{
  scene->clearAll();
}


void editorView::slotButtonTriggered()
{
  QToolButton *act;
  act=qobject_cast<QToolButton *>(sender());
  if (act==fillToolButton)
    {
      scene->apply(editorScene::DFILLCOLOR);
    }
  else if (act==lineToolButton)
    {
      scene->apply(editorScene::DLINECOLOR);
    }
  else if (act==gradientToolButton)
    {
      scene->apply(editorScene::DGRADIENT);
    }
  else
    {
      return;
    }
  update();
}

void editorView::slotColorDialog()
{
  int tp;
  QAction *act;
  QColor c;
  act=qobject_cast<QAction *>(sender());
  tp=act->data().toInt();
  switch(tp)
    {
    case TBFILL:
      c=QColorDialog::getColor(scene->fillColor,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid())
        {
          scene->fillColor=c;
          fillToolButton->setIcon(createColorToolButtonIcon(":/icons/colorfill.png",scene->fillColor));
        }
      break;
    case TBLINE:
      c=QColorDialog::getColor(scene->lineColor,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid())
        {
          scene->lineColor=c;
          lineToolButton->setIcon(createColorToolButtonIcon(":/icons/colorline.png",scene->lineColor));
        }
      break;
    default:
      break;
    }
}

void editorView::slotGradientDialog()
{
  gradientDialog gDiag(this);
  gDiag.selectGradient();
}

void editorView::save(QFile &f,bool templ)
{
  scene->save(f,templ);
  modified=false;
}

bool editorView::open(QFile &f)
{
  if(!scene->load(f)) return false;
  canvasWidth=scene->border.width();
  canvasHeight=scene->border.height();
  changeCanvasSize();
#ifndef QT_NO_DEBUG
  slotDump();
#endif
  return true;
}


void editorView::setTransform()
{
  //	int r=450-rotateDial->value();
  //	if ( r >= 360 )	r-=360;
  scene->hShear=(double)hshearSlider->value()/10.;
  scene->vShear=(double)vshearSlider->value()/10.;
  scene->apply(editorScene::DTRANSFORM);
}

/*! \todo  check if used
*/
void editorView::slotTextReturnPressed(const QString &)
{

}



void editorView::slotShearChanged(int)
{
  QString tmp;
  double shearVal;
  QSlider *sl;
  sl=qobject_cast<QSlider *>(sender());
  shearVal=((double)sl->value())/10;
  tmp=QString("%1").arg(shearVal,4,'g',2,QChar('0'));
  if ( shearVal >= 0 )
    tmp.insert( 0, " " );
  if(sl==hshearSlider)
    {
      hshearLCD->display( tmp );
    }
  else
    {
      vshearLCD->display( tmp );
    }
  setTransform();
}

QIcon editorView::createColorToolButtonIcon(const QString &imageFile, QColor color)
{
  QPixmap pixmap(22, 30);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  QPixmap image(imageFile);
  QRect target(0, 0, 22, 22);
  QRect source(0, 0, 22, 22);
  painter.fillRect(QRect(0, 22, 22, 8), color);
  painter.drawPixmap(target, image, source);
  return QIcon(pixmap);
}

QMenu *editorView::createColorMenu(const char * slot,int type,QString text)
{
  QMenu *colorMenu = new QMenu;
  QAction *action = new QAction(text, this);
  action->setData(type);
  connect(action, SIGNAL(triggered()),this,slot);
  colorMenu->addAction(action);
  return colorMenu;
}

void editorView::slotItemSelected(graphItemBase* ib)
{
  sitemParam p;
  p=ib->getParam();
  if(p.type==graphItemBase::TEXT)
    {
      boldButton->setChecked(p.font.bold());
      underlineButton->setChecked(p.font.underline());
      italicButton->setChecked(p.font.italic());
      fontComboBox->setCurrentIndex(fontComboBox->findText(QFontInfo(p.font).family()));
      fontSizeSpinBox->setValue(p.font.pointSize());
    }
  penWidthSpinBox->setValue(p.pen.widthF());
  hshearSlider->setValue((int)(p.hShear*10));
  vshearSlider->setValue((int)(p.vShear*10));
}


#ifndef QT_NO_DEBUG
void editorView::slotDump()
{
  scene->dumpItems();
}
#endif
