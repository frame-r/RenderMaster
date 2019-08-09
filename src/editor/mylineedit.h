#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit.h>


class MyLineEdit : public QLineEdit
{
	Q_OBJECT
public:
  MyLineEdit(QWidget *parent = 0) : QLineEdit(parent) {}

signals:
  void fileDropped(QString relativePath);

protected:

  void focusInEvent(QFocusEvent *e) {
	QLineEdit::focusInEvent(e);
	selectAll();
  }

  void dragEnterEvent(QDragEnterEvent *e) override;
  void dropEvent(QDropEvent *e) override;
public:


};

#endif // MYLINEEDIT_H
