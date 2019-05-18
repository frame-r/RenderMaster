#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit.h>

class MyLineEdit : public QLineEdit
{
public:
  MyLineEdit(QWidget *parent = 0) : QLineEdit(parent) {}
protected:

  void focusInEvent(QFocusEvent *e) {
	QLineEdit::focusInEvent(e);
	selectAll();
  }

};

#endif // MYLINEEDIT_H
