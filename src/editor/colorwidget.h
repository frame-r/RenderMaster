#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QObject>
#include <QPushButton>
#include <QScopedPointer>
#include <QColorDialog>
#include <QDebug>

class ColorWidget : public QPushButton
{
	QColor _color = QColor(255, 255, 255, 255);
	QColor _initColor;

	Q_OBJECT

private slots:

	void changeColor(QColor nextColor)
	{
		_color = nextColor;

		QString qss = QString("QPushButton {  background-color: %1; }").arg(_color.name());
		setStyleSheet(qss);

		update();

		emit colorChanged(nextColor);
	}

	void onColorDialogAction()
	{
		_initColor = _color;

		QScopedPointer<QColorDialog> colorDialogPtr(new QColorDialog(0));
		QColorDialog * colorDialog = colorDialogPtr.data();
		colorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);

		connect(colorDialog, &QColorDialog::currentColorChanged, this, &ColorWidget::changeColor, Qt::DirectConnection);

		//QColor currentColor = colorDialog->currentColor();
		//currentColor.setAlpha(255);
		//colorDialog->setCurrentColor(currentColor);

		if (colorDialog->exec() == QColorDialog::Accepted)
		{
			changeColor(colorDialog->currentColor());
		}
		else
		{
			changeColor(_initColor);
		}
	}


public:
  ColorWidget(QWidget *parent = 0) : QPushButton(parent)
  {
	  changeColor(_color);

	  connect(this, &QPushButton::clicked, this, &ColorWidget::onColorDialogAction, Qt::DirectConnection);
  }

  void ChangeColor(const QColor& c)
  {
	  _color = c;

	  QString qss = QString("QPushButton {  background-color: %1; }").arg(_color.name());
	  setStyleSheet(qss);

	  update();
  }

signals:
	void colorChanged(QColor color);


//  void mousePressEvent(QMouseEvent *event) override
//  {
//	//QWidget::mousePressEvent(event);
//	//_color = QColorDialog::getColor(_color, this);
//  }
};

#endif // COLORWIDGET_H
