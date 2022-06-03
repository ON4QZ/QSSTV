#ifndef GALLERYWIDGET_H
#define GALLERYWIDGET_H

#include <QWidget>
#include "imageviewer.h"


#define NUMTHUMBS 12

namespace Ui {
    class galleryWidget;
}

class galleryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit galleryWidget(QWidget *parent = 0);
    ~galleryWidget();

  void init();
  void writeSettings();
  void readSettings();
  void changedMatrix();
  void changedMatrix(imageViewer::thumbType itype);
  void putRxImage(QString fn);
  void txImageChanged ();
  void txStockImageChanged();
  QString getTemplateFileName(int);
  const QStringList &getFilenames();
  QString getLastRxImage();

public slots:
  void slotDirChanged(QString);



private:
    Ui::galleryWidget *ui;
    QStringList sl;
};

extern galleryWidget *galleryWidgetPtr;

#endif // GALLERYWIDGET_H
