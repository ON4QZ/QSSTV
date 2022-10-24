#ifndef SCOPEOFFSET_H
#define SCOPEOFFSET_H

#include <QDialog>

namespace Ui {
  class scopeOffset;
  }

class scopeOffset : public QDialog
{
  Q_OBJECT
  
public:
  explicit scopeOffset(QWidget *parent = 0);
  ~scopeOffset();
  void setNumSamples(uint numSamples);
  uint getNumSamples();
  void setOffset(uint offset);
  unsigned int getOffset();
  
private:
  Ui::scopeOffset *ui;
};

#endif // SCOPEOFFSET_H
