#include "modelpropertywidget.h"
#include "ui_modelpropertywidget.h"
#include <QScopedPointer>
#include <QColorDialog>
#include "../colorwidget.h"
#include <QDebug>
#include <QAction>
#include <QCheckBox>
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

void ModelPropertyWidget::destroy_material_group(Material *mat)
{
	//int rows = ui->material_lt->rowCount();
	while (QLayoutItem* item = ui->material_lt->takeAt(0)) {
		delete item->widget();
		delete item;
	}
}

void ModelPropertyWidget::construct_material_group(Material *mat)
{
	destroy_material_group(mat);
	if (!mat)
		return;

	GenericMaterial *genmat = mat->GetGenericMaterial();

	for (size_t i = 0; i < genmat->params_.size(); i++)
	{
		GenericMaterial::Param &p = genmat->params_[i];
		if (p.type != GenericMaterial::PARAM_TYPE::FLOAT)
			continue;

		QSlider *sl = new QSlider();

		int valueInt = mat->GetParamFloat(p.id.c_str()) * sl->maximum();
		sl->setValue(valueInt);

		connect(sl, &QSlider::valueChanged, [p, mat, sl, this](int value)
		{
			float floatValue = (float)value / sl->maximum();
			Material *mat = model_->GetMaterial();

			mat->SetParamFloat(p.id.c_str(), floatValue);

			//QString text;
			//text.sprintf("%6.2f", floatValue);
			//ui->metallic_num_l->setText(text);
		});

		sl->setOrientation(Qt::Orientation::Horizontal);
		ui->material_lt->addRow(tr(p.id.c_str()), sl);
	}

	// color
	for (size_t i = 0; i < genmat->params_.size(); i++)
	{
		GenericMaterial::Param &p = genmat->params_[i];
		if (p.type != GenericMaterial::PARAM_TYPE::COLOR)
			continue;

		ColorWidget *sl = new ColorWidget();

		vec4 current_color = mat->GetParamFloat4(p.id.c_str());
		sl->ChangeColor(EngToQtColor(current_color));

		connect(sl, &ColorWidget::colorChanged, [p, mat, sl, this](QColor next)
		{
			vec4 c = QtColorToEng(next);
			mat->SetParamFloat4(p.id.c_str(), c);
		});

		ui->material_lt->addRow(tr(p.id.c_str()), sl);
	}

	// defines
	for (auto& v : genmat->defs_)
	{
		QCheckBox *cb = new QCheckBox();
		cb->setChecked(mat->GetDef(v.first.c_str()));

		connect(cb, &QCheckBox::toggled, [v, this, mat](bool f)
		{
			mat->SetDef(v.first.c_str(), f);
		});

		ui->material_lt->addRow(tr(v.first.c_str()), cb);
	}

	// textures
	for (auto& v : genmat->textures_)
	{
		TextureLineEdit *l = new TextureLineEdit();
		ui->material_lt->addRow(tr(v.id.c_str()), l);
		l->SetPath(mat->GetTexture(v.id.c_str()));
		l->SetUV(mat->GetUV(v.id.c_str()));

		connect(l, &TextureLineEdit::OnUVChanged, this, [v, mat](const vec4& uv)
		{
			mat->SetUV(v.id.c_str(), uv);
		});
		connect(l, &TextureLineEdit::OnPathChanged, this, [v, mat](const char *path)
		{
			mat->SetTexture(v.id.c_str(), path);
		});
	}
}

ModelPropertyWidget::ModelPropertyWidget(QWidget *parent, Model* m) :
	QWidget(parent),
	model_(m),
	ui(new Ui::ModelPropertyWidget)
{
	ui->setupUi(this);

	ui->mesh_le->setText(QString(model_->GetMeshPath()));

	Material *material = model_->GetMaterial();
	if (material)
		ui->material_le->setText(QString(material->GetId()));

	construct_material_group(material);

	connect(ui->add_btn, &QToolButton::clicked, [this]() -> void
	{
		Material *newMaterial = editor->core->GetMaterialManager()->CreateMaterial("mesh");
		if (newMaterial)
			ui->material_le->setText(QString(newMaterial->GetId()));

		model_->SetMaterial(newMaterial);

		this->construct_material_group(model_->GetMaterial());
	});
}

ModelPropertyWidget::~ModelPropertyWidget()
{
	for (auto& c : connections_)
		QObject::disconnect(c);
	connections_.clear();
	delete ui;
}


