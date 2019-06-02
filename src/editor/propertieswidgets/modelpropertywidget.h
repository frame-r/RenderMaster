#ifndef MODELPROPERTYWIDGET_H
#define MODELPROPERTYWIDGET_H

#include <QWidget>

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

	QMetaObject::Connection albedoTexConn;

	void update_material_group();

public:
	explicit ModelPropertyWidget(QWidget *parent, Model *m);
	~ModelPropertyWidget();

private slots:
	void setAlbedoPath(const char *path);

private:
	Ui::ModelPropertyWidget *ui;

};

#endif // MODELPROPERTYWIDGET_H
