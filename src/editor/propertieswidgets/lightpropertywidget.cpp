#include "lightpropertywidget.h"
#include "ui_lightpropertywidget.h"
#include "light.h"
#include <QComboBox>

static const int spinBoxMultiplier = 10;


LightPropertyWidget::LightPropertyWidget(QWidget *parent, Light *l) :
	QWidget(parent), light(l),
	ui(new Ui::LightPropertyWidget)
{
	ui->setupUi(this);

	// 1. Intensity
	//
	float i = light->GetIntensity();
	ui->intensity_sl->setValue(i * ui->intensity_sl->maximum() / spinBoxMultiplier);
	onIntensityChanged(i);

	connect(ui->intensity_sl, &QSlider::valueChanged, [=](int value)
	{
		float newIntensity = (float)value * spinBoxMultiplier / ui->intensity_sl->maximum();
		light->SetIntensity(newIntensity);
		onIntensityChanged(newIntensity);
	});

	//2. Light type
	//
	ui->type_cb->setCurrentIndex((int)l->GetLightType());
	connect(ui->type_cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int value)
	{
		light->SetLightType((LIGHT_TYPE) value);
	});
}

LightPropertyWidget::~LightPropertyWidget()
{
	delete ui;
}

void LightPropertyWidget::onIntensityChanged(float i)
{
	QString text;
	text.sprintf("%.2f", i);
	ui->intensity_l->setText(text);
}
