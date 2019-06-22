#include "lightpropertywidget.h"
#include "ui_lightpropertywidget.h"
#include "light.h"

static const int spinBoxMultiplier = 10;


LightPropertyWidget::LightPropertyWidget(QWidget *parent, Light *l) :
	QWidget(parent), light(l),
	ui(new Ui::LightPropertyWidget)
{
	ui->setupUi(this);

	float i = light->GetIntensity();
	ui->intensity_sl->setValue(i * ui->intensity_sl->maximum() / spinBoxMultiplier);
	updateIntensityLabel(i);

	connect(ui->intensity_sl, &QSlider::valueChanged, [=](int value)
		{
			float newIntensity = (float)value * spinBoxMultiplier / ui->intensity_sl->maximum();
			light->SetIntensity(newIntensity);
			updateIntensityLabel(newIntensity);
		});
}

LightPropertyWidget::~LightPropertyWidget()
{
	delete ui;
}

void LightPropertyWidget::updateIntensityLabel(float i)
{
	QString text;
	text.sprintf("%.2f", i);
	ui->intensity_l->setText(text);
}
