#ifndef LIGHTPROPERTYWIDGET_H
#define LIGHTPROPERTYWIDGET_H

#include <QWidget>

class Light;

namespace Ui {
class LightPropertyWidget;
}

class LightPropertyWidget : public QWidget
{
	Q_OBJECT

	Light *light;

	void onIntensityChanged(float i);

public:
	explicit LightPropertyWidget(QWidget *parent, Light *l);
	~LightPropertyWidget();

private:
	Ui::LightPropertyWidget *ui;
};

#endif // LIGHTPROPERTYWIDGET_H
