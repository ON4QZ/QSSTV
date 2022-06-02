#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QDialog>
#include <QTime>

class QLCDNumber;

namespace Ui {
  class calibration;
  }

class calibration : public QDialog
{
  Q_OBJECT
  
public:
  explicit calibration(QWidget *parent = 0);
  ~calibration();
  int exec();
  /**
   ** @brief get calibrated receive clock
   *
   * @return double calibrated value of the rxclock
   */
  double getRXClock() {return rxCardClock;}

  /**
   * @brief get calibrated transmit clock
   *
   * @return double calibrated value of the txclock
   */
  double getTXClock(){return txCardClock;}

public slots:
  void hasFinished(int result);
  
private:
  Ui::calibration *ui;
  double rxCardClock;
  double txCardClock;
  bool stopped;
  void init();
  bool start(bool isRX);
  void display(int value,QLCDNumber *dspl);
  bool canceled;
};

#endif // CALIBRATION_H
