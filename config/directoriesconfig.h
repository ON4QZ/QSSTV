#ifndef DIRECTORIES_H
#define DIRECTORIES_H

#include "baseconfig.h"

extern QString rxSSTVImagesPath;
extern QString rxDRMImagesPath;
extern QString txSSTVImagesPath;
extern QString txDRMImagesPath;
extern QString txStockImagesPath;
extern QString templatesPath;
extern QString audioPath;
extern bool saveTXimages;
extern QString docURL;

namespace Ui {
class directoriesConfig;
}

class directoriesConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit directoriesConfig(QWidget *parent = 0);
  ~directoriesConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();

private slots:
  void slotBrowseRxSSTVImagesPath();
  void slotBrowseRxDRMImagesPath();
  void slotBrowseTxSSTVImagesPath();
  void slotBrowseTxDRMImagesPath();
  void slotBrowseTxStockImagesPath();
  void slotBrowseTemplatesPath();
  void slotBrowseAudioPath();
  
private:
  Ui::directoriesConfig *ui;
  void createDir(QString path);
};

#endif // DIRECTORIES_H
