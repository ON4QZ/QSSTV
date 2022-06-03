#ifndef IMAGEMATRIX_H
#define IMAGEMATRIX_H

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QGridLayout>
#include "widgets/imageviewer.h"



class imageMatrix : public QWidget
{
  Q_OBJECT
  
public:
  explicit imageMatrix(QWidget *parent = 0);
  ~imageMatrix();
  void init(int numRows, int numColumns, QString dir, imageViewer::thumbType tt);
  void setupFiles();
  void changed();
  QFileInfoList getFileList(){return fileList;}
  QString getLastFile();
  void setSortFlag(QDir::SortFlags sf) {sortFlags=sf;}

private:
  void getList();
  void displayFiles();
  void setupLayout();
  QVBoxLayout *verticalLayout;
  QHBoxLayout *horizontalLayout;
  QGridLayout *gridLayout;
  QSpacerItem *horizontalSpacer;
  QSpacerItem *horizontalSpacer_2;
  QSpacerItem *horizontalSpacer_3;
  QString dirPath;
  QPushButton *prevPushButton;
  QPushButton *nextPushButton;
  QPushButton *beginPushButton;
  QPushButton *endPushButton;
  QLabel *pageLabel;
  QList<imageViewer *> imageViewerPtrList;
  QFileInfoList fileList;
  int rows;
  int columns;
  int numPages;
  int currentPage;
  QWidget *parentPtr;
  QDir::SortFlags sortFlags;

private slots:
  void slotPrev();
  void slotNext();
  void slotBegin();
  void slotEnd();
  void slotLayoutChanged();

};

#endif // IMAGEMATRIX_H
