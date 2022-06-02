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
#ifndef GRADIENTDIALOG_H
#define GRADIENTDIALOG_H

#include <QtGui>
#include "ui_gradientform.h"

struct sgradientParam
{
	enum gType {NONE,LINEAR,RADIAL,CONICAL};
	sgradientParam()
		{
			type=NONE;
		}
	QColor color1;
	QColor color2;
	QColor color3;
	QColor color4;
	int pos1;
	int pos2;
	int pos3;
	int pos4;
	gType type;
	int direction;
	void load(QDataStream &str);
	void save(QDataStream &str);
};

/**
@author Johan Maes - ON4QZ
*/
class gradientForm;

/** Widget to disply the various canvasItems */
class gradientDialog : public QDialog,private Ui::gradientForm
{
Q_OBJECT
public:
	gradientDialog(QWidget *parent = 0);
	~gradientDialog();
	void readSettings();
	void writeSettings();
	void selectGradient();
	sgradientParam param() {return gParam;}

//	QGradient *constructGradient( QRectF f);
public slots:
	void slotColorDialog();
	void slotUpdate();
private:
	sgradientParam gParam;
	void update();
	QGradient *g;

};

QGradient buildGradient(sgradientParam prm, QRectF f);
#endif

