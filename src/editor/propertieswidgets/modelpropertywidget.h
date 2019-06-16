#ifndef MODELPROPERTYWIDGET_H
#define MODELPROPERTYWIDGET_H

#include <QWidget>
#include <QList>
#include "vector_math.h"

class Material;

template<class T>
class TextureLineEditConcrete;

class Model;

namespace Ui {
class ModelPropertyWidget;
}

class ModelPropertyWidget : public QWidget
{
	Q_OBJECT

	Model *model_;
	QList<QMetaObject::Connection> connections_;

	void destroy_material_group(Material *mat);
	void construct_material_group(Material *mat);

public:
	explicit ModelPropertyWidget(QWidget *parent, Model *m);
	~ModelPropertyWidget();

private:
	Ui::ModelPropertyWidget *ui;

};

#endif // MODELPROPERTYWIDGET_H
