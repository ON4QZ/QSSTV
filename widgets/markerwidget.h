#ifndef MARKERWIDGET_H
#define MARKERWIDGET_H

#include <QLabel>

class markerWidget : public QLabel
{
  Q_OBJECT
public:
  explicit markerWidget(QWidget *parent = 0);
  void setMarkerLabel(QLabel *markerLabel);
  void setMarkers(int mrk1, int mrk2=0, int mrk3=0){ marker1=mrk1;marker2=mrk2;marker3=mrk3;update();}
  
signals:
  
public slots:
private:
  void paintEvent(QPaintEvent *p);
  int marker1;
  int marker2;
  int marker3;
  
};

#endif // MARKERWIDGET_H
