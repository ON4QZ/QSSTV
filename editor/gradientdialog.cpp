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
#include "gradientdialog.h"
//#include "appdefs.h"
#include <QColorDialog>


gradientDialog::gradientDialog(QWidget *parent):QDialog(parent), Ui::gradientForm()
{
	setupUi(this);
	readSettings();
	connect(color1Button,SIGNAL(clicked()),SLOT(slotColorDialog()));
	connect(color2Button,SIGNAL(clicked()),SLOT(slotColorDialog()));
	connect(color3Button,SIGNAL(clicked()),SLOT(slotColorDialog()));
	connect(color4Button,SIGNAL(clicked()),SLOT(slotColorDialog()));
	
	previewLabel->setBackgroundRole(QPalette::Base);
	g=NULL;
	slotUpdate();
	connect(pos1SpinBox,SIGNAL(valueChanged(int)),SLOT(slotUpdate()));
	connect(pos2SpinBox,SIGNAL(valueChanged(int)),SLOT(slotUpdate()));
	connect(pos3SpinBox,SIGNAL(valueChanged(int)),SLOT(slotUpdate()));
	connect(pos4SpinBox,SIGNAL(valueChanged(int)),SLOT(slotUpdate()));
  connect(directionDial,SIGNAL(valueChanged(int)),SLOT(slotUpdate()));
	connect(noGradientButton,SIGNAL(clicked()),SLOT(slotUpdate()));
	connect(linearGradientButton,SIGNAL(clicked()),SLOT(slotUpdate()));
	connect(radialGradientButton,SIGNAL(clicked()),SLOT(slotUpdate()));
	connect(conicalGradientButton,SIGNAL(clicked()),SLOT(slotUpdate()));

}

gradientDialog::~gradientDialog()
{
	writeSettings();
	if(g==NULL) delete g;
}

void gradientDialog::readSettings()
{
	QSettings qSettings;
	qSettings.beginGroup ("Editor");
    gParam.color1 = qSettings.value("gradcolor1", QColor(Qt::red)).value<QColor>();
    gParam.color2 = qSettings.value("gradcolor2", QColor(Qt::green )).value<QColor>();
    gParam.color3 = qSettings.value("gradcolor3", QColor(Qt::yellow)).value<QColor>();
    gParam.color4 = qSettings.value("gradcolor4", QColor(Qt::blue )).value<QColor>();
	gParam.pos1=qSettings.value("gradpos1", 0 ).toInt();
	gParam.pos2=qSettings.value("gradpos2", 0 ).toInt();
	gParam.pos3=qSettings.value("gradpos3", 0 ).toInt();
	gParam.pos4=qSettings.value("gradpos4", 0 ).toInt();
	pos1SpinBox->setValue(gParam.pos1);
	pos2SpinBox->setValue(gParam.pos2);
	pos3SpinBox->setValue(gParam.pos3);
	pos4SpinBox->setValue(gParam.pos4);
	gParam.direction=qSettings.value("graddirection", 0 ).toInt();
  directionDial->setValue((gParam.direction+90)%360);  // 0 degrees is East not North
  directionLCD->display(gParam.direction);
	if(qSettings.value("nogradbutton", 1 ).toBool())
		{
			noGradientButton->setChecked(true);
			gParam.type=sgradientParam::NONE;
		}
	else if(qSettings.value("lineargradbutton", 0 ).toBool())
		{
			linearGradientButton->setChecked(true);
			gParam.type=sgradientParam::LINEAR;;
		}
	else if(qSettings.value("radialgradbutton", 0 ).toBool())
		{
			radialGradientButton->setChecked(true);
			gParam.type=sgradientParam::RADIAL;
		}
	else if(qSettings.value("conicalgradbutton", 0 ).toBool())
		{
			conicalGradientButton->setChecked(true);
			gParam.type=sgradientParam::CONICAL;
		}
	qSettings.endGroup();
}

void gradientDialog::writeSettings()
{
QSettings qSettings;
	qSettings.beginGroup ("Editor" );
	
	qSettings.setValue ("gradcolor1", gParam.color1);
	qSettings.setValue ("gradcolor2", gParam.color2);
	qSettings.setValue ("gradcolor3", gParam.color3);
	qSettings.setValue ("gradcolor4", gParam.color4);
	qSettings.setValue ("gradpos1", gParam.pos1);
	qSettings.setValue ("gradpos2", gParam.pos2);
	qSettings.setValue ("gradpos3", gParam.pos3);
	qSettings.setValue ("gradpos4", gParam.pos4);
	qSettings.setValue ("graddirection", gParam.direction);
	qSettings.setValue ("nogradbutton",noGradientButton->isChecked());
	qSettings.setValue ("lineargradbutton",linearGradientButton->isChecked());
	qSettings.setValue ("radialgradbutton",radialGradientButton->isChecked());
	qSettings.setValue ("conicalgradbutton",conicalGradientButton->isChecked());
	qSettings.endGroup();
}

void sgradientParam::load(QDataStream &str)
{
	int t;
	str >> color1;
	str >> color2;
	str >> color3;
	str >> color4;
	str >> pos1;
	str >> pos2;
	str >> pos3;
	str >>pos4;
	str >> t;
	type=(gType)t;
	str >> direction;
}
void sgradientParam::save(QDataStream &str)
{
	str << color1;
	str << color2;
	str << color3;
	str << color4;
	str << pos1;
	str << pos2;
	str << pos3;
	str << pos4;
	str << (int)type;
	str << direction;
}


void gradientDialog::slotColorDialog()
{
	QColor c;
  QPushButton *act=qobject_cast<QPushButton *>(sender());
  if (act==color1Button)
		{
      c=QColorDialog::getColor(gParam.color1,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid()) gParam.color1=c;
    }
	else if (act==color2Button)
    {
      c=QColorDialog::getColor(gParam.color2,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid()) gParam.color2=c;
    }
	else if (act==color3Button)
    {
      c=QColorDialog::getColor(gParam.color3,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid()) gParam.color3=c;
    }
	else if (act==color4Button)
    {
      c=QColorDialog::getColor(gParam.color4,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid()) gParam.color4=c;
    }
	slotUpdate();
}

/*! \todo split param update from graphic creation
*/
void gradientDialog::slotUpdate()
{
   QString s;
	QPalette palette;
	QBrush brush;
  gParam.direction=(270+directionDial->value())%360; // 0 degrees is East not North
  directionLCD->display(gParam.direction);
	gParam.pos1=pos1SpinBox->value();
	gParam.pos2=pos2SpinBox->value();
	gParam.pos3=pos3SpinBox->value();
	gParam.pos4=pos4SpinBox->value();

  if(noGradientButton->isChecked()) gParam.type=sgradientParam::NONE;
  else if (linearGradientButton->isChecked()) gParam.type=sgradientParam::LINEAR;
  else if (radialGradientButton->isChecked()) gParam.type=sgradientParam::RADIAL;
  else if (conicalGradientButton->isChecked()) gParam.type=sgradientParam::CONICAL;

  s=gParam.color1.name();
  color1Button->setStyleSheet("background-color: "+s+"; border-style: outset; border-width: 2px;border-radius: 10px; border-color: beige; padding: 6px");
  s=gParam.color2.name();
  color2Button->setStyleSheet("background-color: "+s+"; border-style: outset; border-width: 2px;border-radius: 10px; border-color: beige; padding: 6px");
  s=gParam.color3.name();
  color3Button->setStyleSheet("background-color: "+s+"; border-style: outset; border-width: 2px;border-radius: 10px; border-color: beige; padding: 6px");
  s=gParam.color4.name();
  color4Button->setStyleSheet("background-color: "+s+"; border-style: outset; border-width: 2px;border-radius: 10px; border-color: beige; padding: 6px");

	brush.setStyle(Qt::SolidPattern);
	if(gParam.type!=sgradientParam::NONE)
		{
			QBrush br(buildGradient(gParam,previewLabel->rect()));
			palette.setBrush(QPalette::Active, QPalette::Base, br);
		}
	else
		{
			QBrush br(gParam.color1);
			palette.setBrush(QPalette::Active, QPalette::Base, br);
		}
	previewLabel->setPalette(palette);
}


void gradientDialog::selectGradient()
{
	exec();
}

void grSetup (sgradientParam prm,QGradient &g)
{
	g.setColorAt(prm.pos1/100.,prm.color1);
	if(prm.pos2<=prm.pos1) return ;
	g.setColorAt(prm.pos2/100.,prm.color2);
	if(prm.pos3<=prm.pos2) return ;
	g.setColorAt(prm.pos3/100.,prm.color3);
	if(prm.pos4<=prm.pos3) return ;
	g.setColorAt(prm.pos4/100.,prm.color4);
}

QGradient buildGradient(sgradientParam prm, QRectF f)
{
	qreal w=f.width();
	qreal h=f.height();
	qreal d=(double)prm.direction;
	qreal x1,y1,x2,y2;
	qreal temp;
	if(prm.type==sgradientParam::NONE)
		{
			QLinearGradient g(0,0,0,0);
			grSetup(prm,g);
			return g;
		}
	if(prm.type==sgradientParam::LINEAR)
		{
			if(fabs(w/2*tan(M_PI/2-d*M_PI/180))<=(w/2))
				{
					x1=f.x()+w/2-(w/2*tan(M_PI/2-d*M_PI/180.));
					y1=f.y()+h;
					x2=f.x()+w/2+(w/2*tan(M_PI/2-d*M_PI/180.));
					y2=f.y();
					if ((prm.direction>180) && (prm.direction<=359))
						 {
							temp=x1; x1=x2;x2=temp;
							temp=y1; y1=y2;y2=temp;
						}
				}
			else
				{
					x1=f.x();
					y1=f.y()+h/2+(h/2*tan(d*M_PI/180.));
					x2=f.x()+w;
					y2=f.y()+h/2-(h/2*tan(d*M_PI/180.));
					if ((prm.direction>90) && (prm.direction<=270))
						{
							temp=x1; x1=x2;x2=temp;
							temp=y1; y1=y2;y2=temp;
						}
				}
			QLinearGradient g(x1,y1,x2,y2);

			grSetup(prm,g);
			return g;
		}
	else if(prm.type==sgradientParam::RADIAL)
		{
			QRadialGradient g(QPointF(f.x()+f.width()/2,f.y()+f.height()/2),f.width()/2);
			grSetup(prm,g);
			return g;
		}
  else
		{
			QConicalGradient g(QPointF(f.x()+f.width()/2,f.y()+f.height()/2),prm.direction);
			grSetup(prm,g);
			return g;
		}
}
