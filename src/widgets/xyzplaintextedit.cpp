#include "xyzplaintextedit.h"
#include <QDebug>

xyzPlainTextEdit::xyzPlainTextEdit(QWidget *parent) :
  QPlainTextEdit(parent)
{

}

void xyzPlainTextEdit::focusOutEvent(QFocusEvent * event)
{
  emit editingFinished();
  QPlainTextEdit::focusOutEvent(event);
}
