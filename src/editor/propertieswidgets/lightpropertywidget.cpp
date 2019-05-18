#include "lightpropertywidget.h"
#include "ui_lightpropertywidget.h"
#include "light.h"

LightPropertyWidget::LightPropertyWidget(QWidget *parent, Light *l) :
	QWidget(parent), light(l),
	ui(new Ui::LightPropertyWidget)
{
	ui->setupUi(this);

	float i = light->GetIntensity();

	ui->intensity_sl->setValue(i * ui->intensity_sl->maximum());

	QString text;
	text.sprintf("%.2f", i);
	ui->intensity_l->setText(text);

	connect(ui->intensity_sl, &QSlider::valueChanged, [=](int value)
		{
			float floatValue = (float)value / ui->intensity_sl->maximum();

			light->SetIntensity(floatValue);

			QString text;
			text.sprintf("%.2f", floatValue);
			ui->intensity_l->setText(text);
		});
}

LightPropertyWidget::~LightPropertyWidget()
{
	delete ui;
}
