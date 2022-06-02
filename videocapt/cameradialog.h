#ifndef CAMERADIALOG_H
#define CAMERADIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class cameraDialog;
}

class videoCapture;



struct scameraSizes
{
  scameraSizes(int w,int h,QString desc)
  {
    description=desc;
    width=w;
    height=h;
  }
  QString description;
  int width;
  int height;
};

struct sformats
{
  sformats(int frmt,QString desc,QList<scameraSizes> cs)
  {
    format=frmt;
    description=desc;
    cameraSizes=cs;
    sizeIdx=0;
  }
  int format;
  QString description;
  int sizeIdx;
  bool stepwise;
  QList<scameraSizes> cameraSizes;
};

struct scameraDevice
{
  scameraDevice(QString devName,QString devDescription,QString driv,QString bus,QList<sformats> frmts)
  {
    deviceName=devName;
    deviceDescription=devDescription;
    driver=driv;
    busInfo=bus;
    formats=frmts;
    formatIdx=0;
  }
  QString deviceName;
  QString deviceDescription;
  QString driver;
  QString busInfo;
  QList<sformats> formats;
  int formatIdx;
};




class cameraDialog : public QDialog
{
  Q_OBJECT

public:
  explicit cameraDialog(QWidget *parent = 0);
  ~cameraDialog();
  int exec();
  QImage *getImage();
private slots:
  void slotSettings();
  void slotDeviceChanged(int idx);
  void slotFormatChanged(int idx);
  void slotSizeChanged(int idx);


private:

  Ui::cameraDialog *ui;
  void listCameraDevices();
  QList<scameraDevice> cameraList;

  void getCameraInfo(QStringList devList);
  videoCapture *videoCapturePtr;
  bool cameraActive;
  void timerEvent(QTimerEvent *event);
  QString pixelFormatStr(int pixelFormat);
  QList<sformats> getFormatList(int fd);
  void setupFormatComboBox(scameraDevice cd);
  void setupSizeComboBox(sformats frmat);
  int timerID;
  bool restartCapturing(bool first=false);
  void deactivateTimer();


};

#endif // CAMERADIALOG_H

