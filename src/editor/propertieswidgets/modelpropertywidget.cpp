#include "modelpropertywidget.h"
#include "ui_modelpropertywidget.h"
#include <QScopedPointer>
#include <QColorDialog>
#include <QDebug>
#include <QAction>
#include <QToolButton>
#include <QFileDialog>
#include <QSlider>
#include "model.h"
#include "core.h"
#include "material_manager.h"
#include "material.h"
#include "../editorcore.h"
#include "common.h"
#include "../editor_common.h"
#include "../texturelineedit.h"

inline QColor EngToQtColor(const vec4& v)
{
	QColor c = QColor(v.x * 255.0f, v.y * 255.0f, v.z * 255.0f, v.w * 255.0f);
	return c;
}
inline vec4 QtColorToEng(const QColor& c)
{
	vec4 v;
	v.x = c.redF();
	v.y = c.greenF();
	v.z = c.blueF();
	return v;
}

void ModelPropertyWidget::update_material_group()
{
	Material *mat = _obj->GetMaterial();

	if (mat)
	{
		ui->material_gb->setEnabled(1);

		auto path = mat->GetPath();
		ui->material_le->setText(path.c_str());

		vec4 base_color = mat->GetColor();
		ui->color_w->ChangeColor(EngToQtColor(base_color));

		float metallic = mat->GetMetallic();
		int metallicInt = metallic * ui->metallic_sl->maximum();
		ui->metallic_sl->setValue(metallicInt);
		QString text;
		//text.sprintf("%6.2f", metallic);
		//ui->metallic_num_l->setText(text);
		setLabel(ui->metallic_num_l, metallic);

		float rough = mat->GetRoughness();
		int roughInt = rough * ui->roughness_sl->maximum();
		ui->roughness_sl->setValue(roughInt);
		text.sprintf("%6.2f", rough);
		ui->roughness_num_l->setText(text);
	} else
	{
		ui->material_gb->setEnabled(0);
		ui->color_w->ChangeColor(EngToQtColor(vec4(0.5, 0.5, 0.5f, 0.5f)));
	}

	albedoLine->SetObject(mat);
}

ModelPropertyWidget::ModelPropertyWidget(QWidget *parent, Model* m) :
	QWidget(parent),
	_obj(m),
	ui(new Ui::ModelPropertyWidget)
{
	ui->setupUi(this);

	auto *label_3 = new QLabel(ui->material_gb);
	label_3->setText("Albedo");
	ui->formLayout->setWidget(3, QFormLayout::LabelRole, label_3);

	albedoLine = new TextureLineEditConcrete<Material>(ui->material_gb, &Material::SetAlbedoTexName, &Material::GetAlbedoTexName);
	ui->formLayout->setWidget(3, QFormLayout::FieldRole, albedoLine);


	Mesh *mesh = _obj->GetMesh();

	if (mesh)
	{
		ui->mesh_le->setText(QString(mesh->GetPath()));
	}

	update_material_group();

	connect(ui->add_btn, &QToolButton::clicked, [this]() -> void
	{
		Material *m = editor->core->GetMaterialManager()->CreateMaterial();
		_obj->SetMaterial(m);
		this->update_material_group();
	});

//	const char *path;
//	mesh->GetFile(&path);

//	ui->mesh_le->setText(QString(path));

//	update_material_group();

//	connect(ui->material_le, &QLineEdit::editingFinished, [=]()
//		{
//			ui->material_le->clearFocus();
//			 if (_obj)
//			 {
//				 const QString newValue = ui->material_le->text();
//				 _obj->SetMaterial(newValue.toLatin1().data());
//				 update_material_group();
//			 }
//		});

//	// color
	connect(ui->color_w, &ColorWidget::colorChanged, [=](QColor next)
	{
		Material *mat = _obj->GetMaterial();
		if (mat)
		{
			vec4 c = QtColorToEng(next);
			mat->SetColor(c);
		}
	});

//	// metallic
	connect(ui->metallic_sl, &QSlider::valueChanged, [=](int value)
	{
		float floatValue = (float)value / ui->metallic_sl->maximum();
		Material *mat = _obj->GetMaterial();
		if (mat)
			mat->SetMetallic(floatValue);

		QString text;
		text.sprintf("%6.2f", floatValue);
		ui->metallic_num_l->setText(text);
	});

//	// roughness
	connect(ui->roughness_sl, &QSlider::valueChanged, [=](int value)
	{
		float floatValue = (float)value / ui->roughness_sl->maximum();
		Material *mat = _obj->GetMaterial();
		if (mat)
			mat->SetRoughness(floatValue);

		QString text;
		text.sprintf("%6.2f", floatValue);
		ui->roughness_num_l->setText(text);
	});

	Material *mat = _obj->GetMaterial();
	albedoLine->SetObject(mat);

}



ModelPropertyWidget::~ModelPropertyWidget()
{
	delete ui;
}


