/***************************************************************************
                          cw.h  -  QSSTV
                             -------------------
    begin                : Tue Apr 17 22:27:58 CEST 2001
    copyright            : (C) 2001 by Johan Maes ON1MH
    email                : on1mh@pandora.be
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CW_H
#define CW_H
#include <qstring.h>

void initCW(QString cwTxt);
bool sendTextCW(float &tone,float &duration);
float getCWDuration();



#endif
