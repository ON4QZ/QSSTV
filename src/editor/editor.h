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
#ifndef EDITOR_H
#define EDITOR_H

#include <QMainWindow>
#include <QFile>
#include <QComboBox>



class editorView;

/*!
Mainwindow for the image gallery

This editor allows the creation of images and templates.
*/

/*!
@author Johan Maes - ON4QZ
*/
class editor : public QMainWindow
{
	Q_OBJECT

public:
    editor(QWidget *parent=0);
	~editor();
	bool openFile(QString fn);
	void readSettings();
  bool setImage(QImage *im);

public slots:
	void slotFileNew();
	void slotFileOpen();
	void slotFileSave();
	void slotFileSaveImage();
	void slotFileSaveTemplate();
	void slotFileQuit();


private:
	editorView *ev;
	void closeEvent(QCloseEvent *);
	void initActions();
	void initMenubar();

	void writeSettings();
	QAction *fileNew;
	QAction *fileOpen;
	QAction *fileSave;
	QAction *fileSaveImage;
	QAction *fileSaveTemplate;
	QAction *fileQuit;
	QAction *clearAll;
	QAction *copy;
	QAction *paste;
	QAction *deleteAction;
	QMenu *fileMenu;
	QMenu *editMenu;
	QFile localFile;
	QFile externalFile;
};

#endif
