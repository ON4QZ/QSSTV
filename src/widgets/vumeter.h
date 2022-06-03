/***************************************************************************
 *   Copyright (C) 2008 - Giuseppe Cigala                                  *
 *   g_cigala@virgilio.it                                                  *
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

#ifndef VUMETER_H
#define VUMETER_H

#include <QtGui>
#include <math.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif


class vuMeter : public QWidget
{
    Q_OBJECT

    QColor colorBg() const { return colBack; }
    QColor colorValue() const { return colValue;}
    QColor colorLow() const { return colLow;}
    QColor colorHigh() const { return colHigh;}
    double value() const { return val; }
    double minValue() const {return min;}
    double maxValue() const {return max; }

public:

    vuMeter(QWidget *parent = 0);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void setHorizontal(bool h) {horizontal=h;}
    void setLabelText(QString t);


signals:

    void valueChanged(double);

public slots:

    void setColorBg(QColor);
    void setColorValue(QColor);
    void setColorHigh(QColor);
    void setColorMid(QColor);
    void setColorLow(QColor);
    void setColors(QColor,QColor ,QColor);
    void setDivisions(int div) {divisions=div;}
    void setValue(double);
    void setMaximum(double);
    void setMinimum(double);


protected:

    void paintEvent(QPaintEvent *);
    void paintBorder();
    void paintBar();



private:

    double min;
    double max;
    double val;
    QColor colBack;
    QColor colValue;
    QColor colHigh;
    QColor colLow;
    QColor colMid;
    bool horizontal;
    qreal w;
    qreal h;
    int rw;
    int rh;
    int divisions;
    QString labelText;
    double prevValue;


};

#endif
