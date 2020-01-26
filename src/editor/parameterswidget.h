#ifndef PARAMETERSWIDGET_H
#define PARAMETERSWIDGET_H

#include <QWidget>
#include <QSet>
#include "vector_math.h"
#include "myspinbox.h"

class GameObject;

namespace Ui {
class ParametersWidget;
}

class ParametersWidget : public QWidget
{
	Q_OBJECT

	GameObject *g{nullptr}; //TODO: vector of GameObject for multiselection

	bool enabled_;
	vec3 pos_;
	//quat rot_;
	vec3 sc_;

	QWidget *dynamicWidget = nullptr;

	QList<QDoubleSpinBox*> spinBoxes;

public:
	explicit ParametersWidget(QWidget *parent = nullptr);
	~ParametersWidget();

private:
	Ui::ParametersWidget *ui;

	void clearUI();
	void connectPosition(MySpinBox *w, int xyz_offset);
	void connectRotation(MySpinBox *w, int xyz_offset);
	void connectScale(MySpinBox *w, int xyz_offset);

private slots:
	void OnSelectionChanged(QSet<GameObject*>& objects);
	void OnUpdate(float dt);
};

#endif // PARAMETERSWIDGET_H
