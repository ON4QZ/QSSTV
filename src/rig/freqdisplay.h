#ifndef FREQDISPLAY_H
#define FREQDISPLAY_H

#include <QWidget>

namespace Ui {
  class freqDisplay;
  }

class freqDisplay : public QWidget
{
  Q_OBJECT
  
public:
  explicit freqDisplay(QWidget *parent = 0);
  ~freqDisplay();
  void display(double freq);
  
private:
  Ui::freqDisplay *ui;
};

#endif // FREQDISPLAY_H
