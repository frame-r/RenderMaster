#ifndef CAMERAPROPERTYWIDGET_H
#define CAMERAPROPERTYWIDGET_H

#include <QWidget>

namespace Ui {
class CameraPropertyWidget;
}

class CameraPropertyWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CameraPropertyWidget(QWidget *parent = nullptr);
	~CameraPropertyWidget();

private:
	Ui::CameraPropertyWidget *ui;
};

#endif // CAMERAPROPERTYWIDGET_H
