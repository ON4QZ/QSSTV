#ifndef TEMPLATEVIEWER_H
#define TEMPLATEVIEWER_H

#include <QDialog>

namespace Ui {
class templateViewer;
}

class templateViewer : public QDialog
{
  Q_OBJECT

public:
  explicit templateViewer(QWidget *parent = 0);
  ~templateViewer();
  void setImage (QImage *image);

private:
  Ui::templateViewer *ui;
};

#endif // TEMPLATEVIEWER_H
