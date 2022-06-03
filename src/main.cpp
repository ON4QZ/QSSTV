/**************************************************************************
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

#include <QApplication>

#include <QtGui>
#include "appglobal.h"
#include "mainwindow.h"
#include <QPixmap>
#include <QSplashScreen>
#include <QTimer>
#include "dispatcher.h"


QSplashScreen *splash;

int main( int argc, char ** argv )
{

  int result;
  QTimer tm;
  tm.setSingleShot(true);

  QCoreApplication::setOrganizationName(ORGANIZATION);
  QCoreApplication::setApplicationName(APPLICATION);
  QApplication app( argc, argv );
  QPixmap pixmap(":/icons/qsstvsplash.png");
  QSplashScreen splash(pixmap,Qt::WindowStaysOnTopHint);

  splashPtr=&splash;
#ifdef QT_NO_DEBUG
  splash.show();
#endif
  QFont f;
  f.setBold(true);
  f.setPixelSize(20);
  splashPtr->setFont(f);
  splashStr="\n\n\n";
  splashStr+=QString( "Starting %1").arg(qsstvVersion).rightJustified(25,' ')+"\n";
  splash.showMessage (splashStr,Qt::AlignLeft,Qt::white);
  tm.start(100);
  globalInit();
  mainWindowPtr=new mainWindow;
  mainWindowPtr->setWindowIcon(QPixmap(":/icons/qsstv.png"));
  while(1)
  {
    app.processEvents();
    if(!tm.isActive()) break;
   }
  mainWindowPtr->init(); // this must follow show() because window has to be drawn first to determine fftframe window size
  mainWindowPtr->hide();
  tm.start(100);
  while(1)
  {
    app.processEvents();
    if(!tm.isActive()) break;
   }
  splash.finish(mainWindowPtr);
  mainWindowPtr->show();
  mainWindowPtr->startRunning();
  result=app.exec();
  globalEnd();
  return result;
}

