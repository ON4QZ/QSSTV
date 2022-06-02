#ifndef EXTVIEWER_H
#define EXTVIEWER_H

#include <QDialog>
#include <QMovie>

namespace Ui {
class extViewer;
}

class extViewer : public QDialog
{
  Q_OBJECT
  
public:
  explicit extViewer(QWidget *parent = 0);
  ~extViewer();
  void setup(QString fn);
  
private:
  Ui::extViewer *ui;
  int w,h;
  int labelWidth,labelHeight;
  QString fileName;
  QMovie qm;
  QImage im;
  bool activeMovie;
};

#endif // EXTVIEWER_H
