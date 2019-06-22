#include "settings.h"
#include "ui_settings.h"
#include "editorcore.h"
#include "render.h"
#include <QCheckBox>
#include <QComboBox>

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

	ui->diffuse_env_sl->setValue(render->GetDiffuseEnvironemnt() * ui->diffuse_env_sl->maximum());
	setLabel(ui->diffuse_env_l, render->GetDiffuseEnvironemnt());

	ui->specular_env_sl->setValue(render->GetSpecularEnvironemnt() * ui->specular_env_sl->maximum());
	setLabel(ui->specular_env_l, render->GetSpecularEnvironemnt());

	ui->spec_quality_cb->setCurrentIndex(render->GetSpecularQuality());
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

