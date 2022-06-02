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
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H
#include <QLabel>
#include <QSettings>
#include <QEvent>
#include <QMovie>
#include "editor/editor.h"
#include "editor/editorscene.h"

# define IMAGETESTVIEWER
class QMenu;
class QAction;
class editor;
class jp2IO;

class imageViewer : public QLabel
{
  Q_OBJECT
  /*! thumbnail type */
public:
  enum thumbType
  {
    RXIMG, /*!< just for receiver */
    TXIMG, /*!< just for transmitter */
    RXSSTVTHUMB, /*!< thumbnail for receiver. */
    RXDRMTHUMB, /*!< thumbnail for receiver. */
    TXSSTVTHUMB,/*!< thumbnail for transmitter. */
    TXDRMTHUMB,/*!< thumbnail for transmitter. */
    TXSTOCKTHUMB,/*!< thumbnail for transmitter. */
    TEMPLATETHUMB, /*!< thumbnail for template. */
    PREVIEW, /*!< preview tx. */
    EXTVIEW /*!< extViewer. */
  };
  imageViewer(QWidget *parent=0);
  ~imageViewer();


  void init(thumbType tp);
  bool openImage(QString &filename, QString start, bool ask, bool showMessage, bool temitSignal, bool fromCache, bool background);
  bool openImage(QString &filename, bool showMessage, bool emitSignal, bool fromCache,bool background);
  bool openImage(QImage im);
  bool openImage(QByteArray *ba);
  void setParam(QString templateFn,bool usesTemplate,int width=0,int height=0);
  void clear();
  bool reload();


  //  void scale( int w, int h);
  QImage * getImagePtr() {return &sourceImage;}
  bool hasValidImage();
  void setValidImage(bool v)
  {
    validImage=v;
  }

  int diplayedImageBytecount();

  void createImage(QSize sz, QColor fill, bool scale);
  QRgb *getScanLineAddress(int line);
  //  void copy(imageViewer *src);
  void setType(thumbType t);
  QString getFilename() {return imageFileName;}
  QString getCompressedFilename() {return compressedFilename;}
  void enablePopup(bool en) {popupEnabled=en;}
  void displayImage();
  QPoint mapToImage(const QPoint &pos);
  void zoom(QPoint centre, int dlevel);
  void save(QString fileName, QString fmt, bool convertRGB, bool source);
  bool copyToBuffer(QByteArray *ba);
  //  int calcSize(int &sizeRatio);
  uint setSize(int tcommpressSize,bool usesCompression);
  void setAspectMode(Qt::AspectRatioMode mode);
  int getFileSize(){return fileSize;}
  QString toCall;
  QString toOperator;
  QString rsv;
  QString comment1;
  QString comment2;
  QString comment3;
  bool stretch;
  void getOrgSize(int &w,int &h) {w=orgWidth; h=orgHeight;}
  QImage *getDisplayedImage();


  int applyTemplate();


protected:
  void resizeEvent(QResizeEvent *);

public slots:
  void slotToTX();

private slots:
  void slotDelete();
  void slotEdit();
  void slotLoad();
  void slotNew();
  void slotPrint();
  void slotUploadFTP();
  void slotProperties();

  void slotView();
  void slotBGColorChanged();
  void slotZoomIn();
  void slotZoomOut();
  void slotLeftClick();
  void slotJp2ImageDone(bool success, bool fromCache);


signals:
  void layoutChanged();
  void imageChanged();

private:
  QImage displayedImage;
  QImage sourceImage;
  QImage compressedImage;
  QByteArray compressedImageData;


  void mousePressEvent( QMouseEvent *e );
  bool validImage;
  QString imageFileName;
  QString compressedFilename;
  QString imageFilePath;
  thumbType ttype;
  bool popupEnabled;

  QMenu *popup;
  QAction *newAct;
  QAction *loadAct;
  QAction *toTXAct;
  QAction *editAct;
  QAction *printAct;
  QAction *uploadAct;
  QAction *deleteAct;
  QAction *viewAct;
  QAction *propertiesAct;
  QAction *zoomInAct;
  QAction *zoomOutAct;

  //  double psizeRatio;
  int compressSize;   // target size of compressed image
  int fileSize;
  QString format;
  QMovie qm;
  bool activeMovie;
  bool useCompression;
  QString templateFileName;
  Qt::AspectRatioMode aspectRatioMode;
  bool useTemplate;
  int targetWidth;
  int targetHeight;
  int orgWidth;
  int orgHeight;
  QRect view;
  QPoint clickPos;
  QTimer clickTimer;
  QThread *threadIm;
  jp2IO *jp2Ptr;
  bool cacheHit;
  QString tempFilename;
  QImage tempImage;
  bool emitSignal;
  QString cacheFileName;
  bool  processImageDisplay(bool success, bool showMessage, bool fromCache);
#ifdef IMAGETESTVIEWER
  void imageTestViewer(QImage *im, QString infoStr);
#endif



};

#endif
