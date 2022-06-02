#ifndef XYZPLAINTEXTEDIT_H
#define XYZPLAINTEXTEDIT_H

#include <QPlainTextEdit>
#include <QFocusEvent>

class xyzPlainTextEdit : public QPlainTextEdit
{
  Q_OBJECT
public:
  explicit xyzPlainTextEdit(QWidget *parent = 0);
  void focusOutEvent(QFocusEvent * event);

signals:
  void editingFinished();

public slots:

};

#endif // XYZPLAINTEXTEDIT_H
