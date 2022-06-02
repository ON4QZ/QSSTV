#include "templateviewer.h"
#include "ui_templateviewer.h"

templateViewer::templateViewer(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::templateViewer)
{
  ui->setupUi(this);
}

templateViewer::~templateViewer()
{
  delete ui;
}

void templateViewer::setImage(QImage *image)
{
  ui->imageLabel->setPixmap(QPixmap::fromImage(*image));
}


