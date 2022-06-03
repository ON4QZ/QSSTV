#ifndef TEXTDISPLAY_H
#define TEXTDISPLAY_H

#include <QDialog>

namespace Ui {
class textDisplay;
}

class textDisplay : public QDialog
{
  Q_OBJECT
  
public:
  explicit textDisplay(QWidget *parent = 0);
  ~textDisplay();
  void clear();
  void append(QString t);
  
private:
  Ui::textDisplay *ui;
};

#endif // TEXTDISPLAY_H
