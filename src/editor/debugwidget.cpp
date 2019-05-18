#include "debugwidget.h"
#include "ui_debugwidget.h"
#include "editorcore.h"
#include "render.h"
#include "gameobject.h"
#include <QSlider>

static std::vector<QMetaObject::Connection> _connections;

debugwidget::debugwidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::debugwidget)
{
	ui->setupUi(this);

	connect(editor, &EditorCore::OnSelectionChanged, this, &debugwidget::OnSelectionChanged, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineFree, this, &debugwidget::OnEngineFree, Qt::DirectConnection);
	connect(editor, &EditorCore::OnEngineInit, this, &debugwidget::OnEngineInit, Qt::DirectConnection);
}

debugwidget::~debugwidget()
{
	delete ui;
}

void set_vec3(QLineEdit* le, const vec3& v)
{
	char buf[30];
	sprintf(buf, "%.2f %.2f %.2f", v.x, v.y, v.z);
	le->setText(buf);
}
void set_quat(QLineEdit* le, const quat& v)
{
	char buf[30];
	sprintf(buf, "%.2f %.2f %.2f %.2f", v.x, v.y, v.z, v.w);
	le->setText(buf);
}

void debugwidget::OnSelectionChanged(QSet<GameObject *> &objects)
{
	if (objects.size() == 1)
	{
		GameObject *g = *objects.begin();

		set_vec3(ui->pos, g->GetLocalPosition());
		set_quat(ui->rot, g->GetLocalRotation());
		set_vec3(ui->scale, g->GetLocalScale());
		set_vec3(ui->pos_w, g->GetWorldPosition());
		set_quat(ui->rot_w, g->GetWorldRotation());
		set_vec3(ui->scale_w, g->GetWorldScale());

		mat4 local = g->GetLocalTransform();
		mat4 world = g->GetWorldTransform();

		#define SET_II(I, J) \
		{ \
			char buf[10]; \
			sprintf(buf, "%.2f", local.el_2D[I][J]); \
			ui->lineEdit_ ## I ## J ## ->setText(buf); \
			sprintf(buf, "%.2f", world.el_2D[I][J]); \
			ui->lineEdit ## I ## J ## ->setText(buf); \
		}

		SET_II(0,0)
		SET_II(0,1)
		SET_II(0,2)
		SET_II(0,3)
		SET_II(1,0)
		SET_II(1,1)
		SET_II(1,2)
		SET_II(1,3)
		SET_II(2,0)
		SET_II(2,1)
		SET_II(2,2)
		SET_II(2,3)
		SET_II(3,0)
		SET_II(3,1)
		SET_II(3,2)
		SET_II(3,3)


		#undef SET_II
	} else
	{
		ui->pos->setText("-");
		ui->rot->setText("-");
		ui->scale->setText("-");
		ui->pos_w->setText("-");
		ui->rot_w->setText("-");
		ui->scale_w->setText("-");

		#define SET_II(I, J) \
			ui->lineEdit_ ## I ## J ## ->setText("-"); \
			ui->lineEdit ## I ## J ## ->setText("-");

		SET_II(0,0)
		SET_II(0,1)
		SET_II(0,2)
		SET_II(0,3)
		SET_II(1,0)
		SET_II(1,1)
		SET_II(1,2)
		SET_II(1,3)
		SET_II(2,0)
		SET_II(2,1)
		SET_II(2,2)
		SET_II(2,3)
		SET_II(3,0)
		SET_II(3,1)
		SET_II(3,2)
		SET_II(3,3)

		#undef SET_II
	}
}

void debugwidget::OnEngineFree(Core *c)
{
	for (auto& conn : _connections)
		QObject::disconnect(conn);
	_connections.clear();
}

void debugwidget::OnEngineInit(Core *c)
{
	_connections.emplace_back(connect(ui->slider_0, &QSlider::valueChanged, this, &debugwidget::sliderOChanged, Qt::DirectConnection));
	_connections.emplace_back(connect(ui->slider_1, &QSlider::valueChanged, this, &debugwidget::slider1Changed, Qt::DirectConnection));
}

void debugwidget::sliderOChanged(int i)
{
	Render *render = editor->core->GetRender();
	render->SetDebugParam((float)ui->slider_0->value()/ui->slider_0->maximum());
}

void debugwidget::slider1Changed(int i)
{
	Render *render = editor->core->GetRender();
	render->SetDebugParam1((float)ui->slider_1->value()/ui->slider_1->maximum());
}
