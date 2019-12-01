#include "settings.h"
#include "ui_settings.h"
#include "editorcore.h"
#include "render.h"
#include <QCheckBox>
#include <QComboBox>
#include "mylineedit.h"

static std::vector<QMetaObject::Connection> _connections;

Settings::Settings(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Settings)
{
	ui->setupUi(this);

	connect(editor, &EditorCore::OnEngineFree, this, &Settings::OnEngineFree, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineInit, this, &Settings::OnEngineInit, Qt::DirectConnection);

}

Settings::~Settings()
{
	delete ui;
}

bool Settings::isWireframeAntialiasing() const
{
	Render *render = editor->core->GetRender();
	return render->IsWireframeAA();
}

void Settings::OnEngineFree(Core *c)
{
	for (auto& conn : _connections)
		QObject::disconnect(conn);
	_connections.clear();
}

void Settings::OnEngineInit(Core *c)
{
	Render *render = editor->core->GetRender();

	_connections.emplace_back(connect(ui->diffuse_env_sl, &QSlider::valueChanged, this, &Settings::sliderOChanged, Qt::DirectConnection));
	_connections.emplace_back(connect(ui->specular_env_sl, &QSlider::valueChanged, this, &Settings::slider1Changed, Qt::DirectConnection));
	_connections.emplace_back(connect(ui->spec_quality_cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx)->void
	{
		Render *render = editor->core->GetRender();
		render->SetSpecularQuality(ui->spec_quality_cb->currentIndex());
	}));

	_connections.emplace_back(connect(ui->taa_cb, &QCheckBox::toggled, [this](bool value)->void
	{
		Render *render = editor->core->GetRender();
		render->SetTAA(value);
	}));

	_connections.emplace_back(connect(ui->wireframe_aa, &QCheckBox::toggled, [this](bool value)->void
	{
		Render *render = editor->core->GetRender();
		render->SetWireframeAA(value);
	}));

	_connections.emplace_back(connect(ui->wireframe_, &QCheckBox::toggled, [this](bool value)->void
	{
		Render *render = editor->core->GetRender();
		render->SetWireframe(value);
	}));

	_connections.emplace_back(connect(ui->env_le, &QLineEdit::returnPressed, [this]()->void
	{
		Render *render = editor->core->GetRender();
		render->SetEnvironmentTexturePath(ui->env_le->text().toLatin1());
	}));

	_connections.emplace_back(QObject::connect(ui->env_le, &MyLineEdit::fileDropped,
	[this](QString relativePath)
	{
		if (isTexture(relativePath))
		{
			ui->env_le->setText(relativePath);
			Render *render = editor->core->GetRender();
			render->SetEnvironmentTexturePath(ui->env_le->text().toLatin1());
		}
	}));

	ui->diffuse_env_sl->setValue(render->GetDiffuseEnvironemnt() * ui->diffuse_env_sl->maximum());
	setLabel(ui->diffuse_env_l, render->GetDiffuseEnvironemnt());

	ui->specular_env_sl->setValue(render->GetSpecularEnvironemnt() * ui->specular_env_sl->maximum());
	setLabel(ui->specular_env_l, render->GetSpecularEnvironemnt());

	ui->spec_quality_cb->setCurrentIndex(render->GetSpecularQuality());

	ui->taa_cb->setChecked(render->IsTAA());
	ui->wireframe_aa->setChecked(render->IsWireframeAA());
	ui->wireframe_->setChecked(render->IsWireframe());

	ui->env_le->setText(render->GetEnvironmentTexturePath());
}

void Settings::sliderOChanged(int i)
{
	Render *render = editor->core->GetRender();

	render->SetDiffuseEnvironemnt((float)ui->diffuse_env_sl->value()/ui->diffuse_env_sl->maximum());
	setLabel(ui->diffuse_env_l, render->GetDiffuseEnvironemnt());
}

void Settings::slider1Changed(int i)
{
	Render *render = editor->core->GetRender();

	render->SetSpecularEnvironemnt((float)ui->specular_env_sl->value()/ui->specular_env_sl->maximum());
	setLabel(ui->specular_env_l, render->GetSpecularEnvironemnt());

}

