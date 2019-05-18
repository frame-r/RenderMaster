#include "camerapropertywidget.h"
#include "ui_camerapropertywidget.h"

CameraPropertyWidget::CameraPropertyWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CameraPropertyWidget)
{
	ui->setupUi(this);
}

CameraPropertyWidget::~CameraPropertyWidget()
{
	delete ui;
}
