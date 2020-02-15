#include "parameterswidget.h"
#include "ui_parameterswidget.h"
#include "propertieswidgets/modelpropertywidget.h"
#include "propertieswidgets/camerapropertywidget.h"
#include "propertieswidgets/lightpropertywidget.h"
#include "editorcore.h"
#include "gameobject.h"
#include "model.h"
#include "light.h"

static std::vector<QMetaObject::Connection> _connections;

ParametersWidget::ParametersWidget(QWidget *parent) :
	QWidget(parent)
{
	ui = new Ui::ParametersWidget;
	ui->setupUi(this);
	connect(editor, &EditorCore::OnSelectionChanged, this, &ParametersWidget::OnSelectionChanged, Qt::DirectConnection);
	connect(editor, &EditorCore::OnUpdate, this, &ParametersWidget::OnUpdate, Qt::DirectConnection);

	clearUI();
}

ParametersWidget::~ParametersWidget()
{
	delete ui;
}

void ParametersWidget::clearUI()
{
	ui->lineEdit->clear();
	ui->lineEdit->setEnabled(false);
	ui->enabled_cb->setEnabled(false);

	spinBoxes.append(ui->pos_x_sb);
	spinBoxes.append(ui->pos_y_sb);
	spinBoxes.append(ui->pos_z_sb);
	spinBoxes.append(ui->rot_x_sb);
	spinBoxes.append(ui->rot_y_sb);
	spinBoxes.append(ui->rot_z_sb);
	spinBoxes.append(ui->scale_x_sb);
	spinBoxes.append(ui->scale_y_sb);
	spinBoxes.append(ui->scale_z_sb);

	for (auto *sp : spinBoxes)
		sp->clear();

	ui->groupBox->setEnabled(false);
}

void ParametersWidget::OnSelectionChanged(QSet<GameObject *> &objects)
{
	if (objects.size() == 1)
	{
		for (auto *sp : spinBoxes)
			sp->blockSignals(true);

		g = *objects.begin();

		ui->lineEdit->setEnabled(true);
		ui->groupBox->setEnabled(true);
		ui->enabled_cb->setEnabled(true);

		ui->enabled_cb->setChecked(g->IsEnabled());

		const char *name = g->GetName();
		ui->lineEdit->setText(name);

		pos_ = g->GetWorldPosition();

		ui->pos_x_sb->setValue(static_cast<double>(pos_.x));
		ui->pos_y_sb->setValue(static_cast<double>(pos_.y));
		ui->pos_z_sb->setValue(static_cast<double>(pos_.z));

		quat rot = g->GetWorldRotation();
		vec3 euler_ = rot.ToEuler();
		ui->rot_x_sb->setValue(static_cast<double>(euler_.x));
		ui->rot_y_sb->setValue(static_cast<double>(euler_.y));
		ui->rot_z_sb->setValue(static_cast<double>(euler_.z));

		sc_ = g->GetWorldScale();
		ui->scale_x_sb->setValue(static_cast<double>(sc_.x));
		ui->scale_y_sb->setValue(static_cast<double>(sc_.y));
		ui->scale_z_sb->setValue(static_cast<double>(sc_.z));

		connectPosition(ui->pos_x_sb, 0);
		connectPosition(ui->pos_y_sb, 1);
		connectPosition(ui->pos_z_sb, 2);

		connectRotation(ui->rot_x_sb, 0);
		connectRotation(ui->rot_y_sb, 1);
		connectRotation(ui->rot_z_sb, 2);

		connectScale(ui->scale_x_sb, 0);
		connectScale(ui->scale_y_sb, 1);
		connectScale(ui->scale_z_sb, 2);

		// Dynamic block

		if (dynamicWidget)
		{
			ui->dynamic_block->removeWidget(dynamicWidget);
			delete dynamicWidget;
			dynamicWidget = nullptr;
		}

		switch(g->GetType())
		{
			case OBJECT_TYPE::MODEL: dynamicWidget = new ModelPropertyWidget(this, static_cast<Model*>(g)); break;
			case OBJECT_TYPE::LIGHT: dynamicWidget = new LightPropertyWidget(this, static_cast<Light*>(g)); break;
			default: break;
			//case OBJECT_TYPE::CAMERA: dynamicWidget = new CameraPropertyWidget(this, static_cast<Camera*>(g)); break;
		}

		if (dynamicWidget)
			ui->dynamic_block->addWidget(dynamicWidget);

		ui->verticalSpacer->changeSize(0,0,QSizePolicy::Maximum,QSizePolicy::Maximum);

		for (auto *sp : spinBoxes)
			sp->blockSignals(false);
	} else
	{
		for (auto& conn : _connections)
			QObject::disconnect(conn);

		_connections.clear();
		g = nullptr;
		clearUI();

		if (dynamicWidget)
		{
			ui->dynamic_block->removeWidget(dynamicWidget);
			delete dynamicWidget;
			dynamicWidget = nullptr;
		}

		ui->verticalSpacer->changeSize(0,0,QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	}
}

inline bool Approximately(float l, float r)
{
	const float eps = 0.000001f;
	return std::abs(l - r) < eps;
}

void qd(quat& q)
{
	qDebug() << q.x << q.y << q.z << q.w;
}

void ParametersWidget::OnUpdate(float)
{
	if (!g)
		return;

	if (enabled_ != g->IsEnabled())
		ui->enabled_cb->setChecked(g->IsEnabled());
	_connections.emplace_back(connect(ui->enabled_cb, &QCheckBox::toggled, [this](bool v) {  g->SetEnabled(v); }));

	// position
	{
		vec3 p = g->GetWorldPosition();

		#define SET_VAL(X, Y, F) \
		if (!Approximately(X, Y)) \
		{ \
			Y = X; \
			ui-> F ->setValue(static_cast<double>(X)); \
		}

		SET_VAL(p.x, pos_.x, pos_x_sb)
		SET_VAL(p.y, pos_.y, pos_y_sb)
		SET_VAL(p.z, pos_.z, pos_z_sb)
	}

	// rotation
	{
		quat rot = g->GetWorldRotation();

		float x = static_cast<float>(ui->rot_x_sb->value());
		float y = static_cast<float>(ui->rot_y_sb->value());
		float z = static_cast<float>(ui->rot_z_sb->value());

		vec3 euler = {x, y, z};
		quat oldRot = quat(euler);

		vec3 e = rot.ToEuler();

		if (!rot.IsSameRotation(oldRot))
		{
			ui->rot_x_sb->setValue(static_cast<double>(e.x));
			ui->rot_y_sb->setValue(static_cast<double>(e.y));
			ui->rot_z_sb->setValue(static_cast<double>(e.z));
		}
	}

	// scale
	{
		vec3 p = g->GetWorldScale();

		#define SET_VAL(X, Y, F) \
		if (!Approximately(X, Y)) \
		{ \
			Y = X; \
			ui-> F ->setValue(static_cast<double>(X)); \
		}

		SET_VAL(p.x, sc_.x, scale_x_sb)
		SET_VAL(p.y, sc_.y, scale_y_sb)
		SET_VAL(p.z, sc_.z, scale_z_sb)
	}


}

inline bool getFloatSpinbox(const QString& str, float &ret)
{
	bool ok;
	QLocale german(QLocale::English);
	ret = german.toFloat(str, &ok);
	return ok;
}

void ParametersWidget::connectPosition(MySpinBox *w, int i)
{
	auto conn = connect(w, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged),
	[=](const QString &newValue)
	{
		if (!g)
			return;

		float newValueFloat;
		if (!getFloatSpinbox(newValue, newValueFloat))
			return;

		pos_.xyz[i] = newValueFloat;
		g->SetWorldPosition(pos_);

	});
	_connections.emplace_back(conn);
}

void ParametersWidget::connectRotation(MySpinBox *w, int i)
{
	auto conn = connect(w, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged),
	[i, this](const QString &)
	{
		if (!g)
			return;

		quat rot = g->GetWorldRotation();

		float vals[3];
		vals[0] = static_cast<float>(ui->rot_x_sb->value());
		vals[1] = static_cast<float>(ui->rot_y_sb->value());
		vals[2] = static_cast<float>(ui->rot_z_sb->value());

		vec3 euler = {vals[0], vals[1], vals[2]};
		quat newRot = quat(euler);

		if (!newRot.IsSameRotation(rot))
			g->SetWorldRotation(newRot);
	});
	_connections.emplace_back(conn);
}

void ParametersWidget::connectScale(MySpinBox *w, int i)
{
	auto conn = connect(w, QOverload<const QString&>::of(&QDoubleSpinBox::valueChanged),
	[i, this](const QString &newValue)
	{
		if (!g)
			return;

		float newValueFloat;
		if (!getFloatSpinbox(newValue, newValueFloat))
			return;

		sc_.xyz[i] = newValueFloat;
		g->SetWorldScale(sc_);
	});
	_connections.emplace_back(conn);
}
