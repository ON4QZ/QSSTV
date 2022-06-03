#include "imagematrix.h"
#include <QDir>
#include <QDebug>
#include <iostream>

#define MINCOLSIZE 32
#define MINROWSIZE 26
#define MAXCOLSIZE 64
#define MAXROWSIZE 52


imageMatrix::imageMatrix(QWidget *parent) :  QWidget(parent)
{

  parentPtr=parent;
  parentPtr->resize(511, 300);
  verticalLayout = NULL;
  horizontalLayout=NULL;
  sortFlags=QDir::Time;
}

imageMatrix::~imageMatrix()
{
// if(verticalLayout!=NULL) delete verticalLayout;
// if( horizontalLayout!=NULL) delete horizontalLayout;
}

void imageMatrix::setupLayout()
{
  if(verticalLayout!=NULL) delete verticalLayout;
  verticalLayout = new QVBoxLayout(parentPtr);
  verticalLayout->setObjectName(QString::fromUtf8("vt1"));
  verticalLayout->setSpacing(2);
  verticalLayout->setContentsMargins(1, 1, 1, 1);
  gridLayout = new QGridLayout();
  gridLayout->setSpacing(1);
  gridLayout->setObjectName(QString::fromUtf8("gridLo"));
  gridLayout->setSizeConstraint(QLayout::SetNoConstraint);
  verticalLayout->addLayout(gridLayout);

  horizontalLayout = new QHBoxLayout();
  horizontalLayout->setObjectName(QString::fromUtf8("htl"));
  horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  horizontalLayout->addItem(horizontalSpacer);

  beginPushButton = new QPushButton(this);
  beginPushButton->setObjectName(QString::fromUtf8("beginPushButton"));
  QIcon icon2;
  icon2.addFile(QString::fromUtf8(":/icons/doubleleft.png"), QSize(), QIcon::Normal, QIcon::Off);
  beginPushButton->setIcon(icon2);
  horizontalLayout->addWidget(beginPushButton);

  prevPushButton = new QPushButton(this);
  prevPushButton->setObjectName(QString::fromUtf8("prevPushButton"));
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/icons/left.png"), QSize(), QIcon::Normal, QIcon::Off);
  prevPushButton->setIcon(icon);
  horizontalLayout->addWidget(prevPushButton);
  pageLabel=new QLabel;
  horizontalLayout->addWidget(pageLabel);

//  horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
//  horizontalLayout->addItem(horizontalSpacer_2);
  nextPushButton = new QPushButton(this);
  nextPushButton->setObjectName(QString::fromUtf8("nextPushButton"));
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/icons/start.png"), QSize(), QIcon::Normal, QIcon::Off);
  nextPushButton->setIcon(icon1);
  horizontalLayout->addWidget(nextPushButton);

  endPushButton = new QPushButton(this);
  endPushButton->setObjectName(QString::fromUtf8("endPushButton"));
  QIcon icon3;
  icon3.addFile(QString::fromUtf8(":/icons/doubleright.png"), QSize(), QIcon::Normal, QIcon::Off);
  endPushButton->setIcon(icon3);
  horizontalLayout->addWidget(endPushButton);

  horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  horizontalLayout->addItem(horizontalSpacer_3);
  verticalLayout->addLayout(horizontalLayout);
  connect(prevPushButton,SIGNAL(clicked()),SLOT(slotPrev()));
  connect(nextPushButton,SIGNAL(clicked()),SLOT(slotNext()));
  connect(beginPushButton,SIGNAL(clicked()),SLOT(slotBegin()));
  connect(endPushButton,SIGNAL(clicked()),SLOT(slotEnd()));
}


void imageMatrix::init(int numRows, int numColumns, QString dir,imageViewer::thumbType tt)
{
  int i,j;
  rows=numRows;
  columns=numColumns;
  dirPath=dir;
  imageViewer *imv;
  setupLayout();
  for(i=0;i<rows;i++)
    {
      for(j=0;j<columns;j++)
        {
          imv = new imageViewer(this);
          imv->setType(tt);
          gridLayout->addWidget(imv, i, j, 1, 1);
          connect(imv,SIGNAL(layoutChanged()),SLOT(slotLayoutChanged()));
        }
    }
  for (i=0;i<rows;i++)
    {
      gridLayout->setRowMinimumHeight(i,MINROWSIZE);
      gridLayout->setRowStretch(i,0);
    }

  for (i=0;i<columns;i++)
    {
      gridLayout->setColumnMinimumWidth(i,MINCOLSIZE);
      gridLayout->setColumnStretch(i,1);
    }
  currentPage=0;
  getList();
//  displayFiles();
}


void imageMatrix::getList()
{


  QDir dir(dirPath);
  dir.setFilter(QDir::Files | QDir::NoSymLinks);
  dir.setSorting(sortFlags);
  fileList = dir.entryInfoList();
  numPages=ceil((double)fileList.count()/(double)(rows*columns));
  if(numPages==0) numPages=1;
  slotBegin();
}

QString imageMatrix::getLastFile()
{
  if (fileList.count()>0)
    {
      return fileList.last().absoluteFilePath();
    }
  else return QString();
}

void imageMatrix::displayFiles()
{
  int i,j,k;

  QString tempStr;
  int offset=currentPage*rows*columns;
  pageLabel->setText(QString("   Page %1 of %2").arg(currentPage+1).arg(numPages).leftJustified(17,' '));
  for(i=0;i<rows;i++)
    {
      for(j=0;j<columns;)
        {
          k=offset+i*columns+j;
          if(k>=fileList.count())
            {
              ((imageViewer *)gridLayout->itemAtPosition(i,j)->widget())->clear();
              j++;
            }
          else
            {
           tempStr=fileList.at(k).absoluteFilePath();
           if(((imageViewer *)gridLayout->itemAtPosition(i,j)->widget())->openImage(tempStr,false,false,true,true))
             {
                j++;
             }
           else
             {
               fileList.removeAt(k);
             }
            }
        }
    }
}

void imageMatrix::changed()
{
    getList();
//    displayFiles();
}


void imageMatrix::slotPrev()
{
  if(currentPage!=0) currentPage--;
  displayFiles();
}

void imageMatrix::slotNext()
{
  currentPage++;
  if(currentPage>=numPages)
    {
      currentPage--;
    }
  displayFiles();
}

void imageMatrix::slotBegin()
{
  currentPage=0;
  displayFiles();
}

void imageMatrix::slotEnd()
{
  currentPage=numPages-1;
  if(currentPage<0) currentPage=0;
  displayFiles();
}

void imageMatrix::slotLayoutChanged()
{
  int curPag=currentPage;
  getList();
  if(curPag>=numPages)
    {
      if(curPag>0) curPag--;
    }
  currentPage=curPag;
  displayFiles();
}
