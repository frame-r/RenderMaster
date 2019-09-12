#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "renderwidget.h"
#include "icontitlewidget.h"
#include "consolewidget.h"
#include "scenewidget.h"
#include "editorcore.h"
#include "debugwidget.h"
#include "parameterswidget.h"
#include "render.h"
#include "projectwidget.h"
#include "selectmeshdialog.h"
#include "settings.h"

#include "resource_manager.h"

#include "ads/API.h"
#include "ads/ContainerWidget.h"
#include "ads/SectionContent.h"

#include <QComboBox>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QShortcut>
#include <QCheckBox>

#define MAIN_WINDOW_LT "MainWindow"
#define WINDOWS_LT "Windows"
#define THEME_LT "Theme"

static ADS_NS::ContainerWidget* _container;

static QByteArray loadADS(const QString& fname);
static void saveADS(const QString& fname, const QByteArray& ba);
static void toggleWindow(int i, bool show);
static void updateMenuButtons();

struct WindowData
{
	ADS_NS::SectionContent::RefPtr ptr;
	QAction *act;
	QWidget *w;
	bool visible;
};

static WindowData _windows[WINDOWS_NUM];


enum ENGINE_ACTIONS
{
	LOAD = 0,
	PROFILER,
	//RELOAD_CORE_RENDER,
	ENGINE_ACTIONS_NUM,
};

static QAction* engineActions[ENGINE_ACTIONS_NUM] = {};


template<typename T>
static T* createWidget(const QString& name, int id, ADS_NS::DropArea drop, QMenu *menuLayout, MainWindow* wnd, bool hidden = true)
{
	T* w = new T(wnd);

	auto sc = ADS_NS::SectionContent::newSectionContent(name, _container, new IconTitleWidget(QIcon(), name), w);
	_windows[id].ptr = sc;
	_windows[id].w = w;
	_container->addSectionContent(sc, nullptr, drop);

	if (hidden)
		_container->hideSectionContent(sc);

	QAction *act = new QAction(wnd);
	_windows[id].act = act;
	act->setText(name);
	act->setCheckable(true);
	act->setChecked(_windows[id].visible);
	menuLayout->addAction(act);
	wnd->connect(act, &QAction::triggered, [id](bool v) {toggleWindow(id, v);});

	return w;
}

void load_theme(QString name)
{
	QFile File(name);
	File.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(File.readAll());
	qApp->setStyleSheet(StyleSheet);
	File.close();
}

void MainWindow::applyDarkStyle()
{
	theme = DARK;
	load_theme(DARK_THEME);
}

void MainWindow::applyLightStyle()
{
	theme = LIGHT;
	load_theme(LIGHT_THEME);
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	editor->window = this;

	ui->setupUi(this);

	installEventFilter(this);

	// restore window geometry
	_container = new ADS_NS::ContainerWidget();
	setCentralWidget(_container);
	resize(800, 600);
	restoreGeometry(loadADS(MAIN_WINDOW_LT));

	// deserialize theme
	QByteArray a = loadADS(THEME_LT);
	char t = (char)a[0];
	theme = (THEME)t;
	if (theme == LIGHT)
		applyLightStyle();
	else
		applyDarkStyle();


	//------Layout menu and main windows-------


	// create console
	createWidget<ConsoleWidget>(QString("Console"), CONSOLE, DropArea::BottomDropArea, ui->menuLayout, this);

	// scene tree
	SceneWidget *scene = createWidget<SceneWidget>(QString("Scene"), SCENETREE, DropArea::BottomDropArea, ui->menuLayout, this);

	createWidget<debugwidget>(QString("DebugWidget"), DEBUG_WIDGET, DropArea::LeftDropArea, ui->menuLayout, this);
	createWidget<ParametersWidget>(QString("ParametersWidget"), PARAMETERS, DropArea::LeftDropArea, ui->menuLayout, this);
	createWidget<ProjectWidget>(QString("ProjectWidget"), PROJECT_WIDGET, DropArea::LeftDropArea, ui->menuLayout, this);
	createWidget<Settings>(QString("Settings"), SETTINGS, DropArea::LeftDropArea, ui->menuLayout, this);

	// create render widgets
	for (int i = 0; i < VIEWPORT_NUM; i++)
	{
		RenderWidget *r = createWidget<RenderWidget>(QString("Viewport %1").arg(i), VIEWPORT_0 + i, ADS_NS::RightDropArea, ui->menuLayout, this, i != 0);
		r->setObjectName("RenderWidget");
	}


	// create separator
	ui->menuLayout->addSeparator();

	// create theme buttons
	QMenu *themeMenu = ui->menuLayout->addMenu(tr("&Theme"));

	{
		QAction *act = new QAction(this);
		act->setText("Light");
		themeMenu->addAction(act);
		connect(act, &QAction::triggered, [this]() { applyLightStyle(); });
	}
	{
		QAction *act = new QAction(this);
		act->setText("Dark");
		themeMenu->addAction(act);
		connect(act, &QAction::triggered, [this]() { applyDarkStyle(); });
	}

	// restore all windows geometry
	_container->restoreState(loadADS(WINDOWS_LT));


	// set shortcuts
	QShortcut *shortcut_dark = new QShortcut(QKeySequence(Qt::Key_F5), this);
	QObject::connect(shortcut_dark, &QShortcut::activated, this, &MainWindow::applyDarkStyle);

	QShortcut *shortcut_light = new QShortcut(QKeySequence(Qt::Key_F6), this);
	QObject::connect(shortcut_light, &QShortcut::activated, this, &MainWindow::applyLightStyle);

	QShortcut *shortcut_s_reload = new QShortcut(QKeySequence(Qt::Key_F7), this);
	QObject::connect(shortcut_s_reload, &QShortcut::activated, []() { editor->ReloadShaders(); });

	QShortcut *shortcut_delete = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	QObject::connect(shortcut_delete, &QShortcut::activated, scene, &SceneWidget::RemoveSelectedObjects);

	QShortcut *shortcut_duplicate = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this);
	QObject::connect(shortcut_duplicate, &QShortcut::activated, editor, &EditorCore::CloneSelectedGameObject);

	QShortcut *shortcut_save = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this);
	QObject::connect(shortcut_save, &QShortcut::activated, []() { editor->SaveWorld(); });

	QShortcut *shortcut_focus = new QShortcut(QKeySequence(Qt::Key_F), this);
	QObject::connect(shortcut_focus, &QShortcut::activated, []() { editor->Focus(); });


	//------Engine menu-------

	engineActions[LOAD] = ui->actionLoad;
	engineActions[LOAD]->setCheckable(true);
	engineActions[LOAD]->setChecked(editor->IsEngineLoaded());
	connect(engineActions[LOAD], &QAction::triggered, [](bool v)
	{
		if(v)
			editor->LoadEngine();
		else
			editor->UnloadEngine();
	});

	engineActions[PROFILER] = ui->actionProfiler;
	engineActions[PROFILER]->setCheckable(true);
	engineActions[PROFILER]->setChecked(editor->core ? editor->core->IsProfiler() : false);
	connect(engineActions[PROFILER], &QAction::triggered, [](bool v)
	{
		if (editor->core)
		{
			editor->core->SetProfiler(v);
		}
	});

	QComboBox *view_cb = new QComboBox(this);
	view_cb->addItem("Final");
	view_cb->addItem("Normal");
	view_cb->addItem("Albedo");
	view_cb->addItem("Diffuse light (only analytic)");
	view_cb->addItem("Specular light (only analytic)");
	view_cb->addItem("Velocity");
	view_cb->addItem("Color reprojection");
	ui->mainToolBar->addWidget(view_cb);
	connect(view_cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [view_cb](int idx)->void
	{
		Render *render = editor->core->GetRender();
		render->SetViewMode(static_cast<VIEW_MODE>(view_cb->currentIndex()));
	});

	switch_button(ui->actionselect);

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, []() { updateMenuButtons(); });
	timer->start(700);

}

//void MainWindow::keyPressEvent(QKeyEvent* event)
//{
//	if (event->key() == Qt::Key_Alt) { qDebug() << "keyPressEvent"; keyAlt = 1; }
//	QMainWindow::keyPressEvent(event);
//}

//void MainWindow::keyReleaseEvent(QKeyEvent* event)
//{
//	if (event->key() == Qt::Key_Alt) { qDebug() << "keyReleaseEvent";keyAlt = 0; }
//	QMainWindow::keyReleaseEvent(event);
//}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		   emit onKeyPressed((Qt::Key)keyEvent->key());
	}else if (event->type() == QEvent::KeyRelease)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		   emit onKeyReleased((Qt::Key)keyEvent->key());

	}
	return QMainWindow::eventFilter(obj, event);
}

MainWindow::~MainWindow()
{
	delete ui;
}

WId MainWindow::getWindowHandle()
{
	QWidget *render_view = findChild<RenderWidget*>("RenderWidget");
	return render_view->winId();
}

Settings *MainWindow::GetSettings()
{
	WindowData& data = _windows[SETTINGS];
	Settings *s = static_cast<Settings*>(data.w);
	return s;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	Q_UNUSED(e);

	QByteArray a;
	a.resize(1);
	a[0] = theme;
	saveADS(THEME_LT, a);

	saveADS(MAIN_WINDOW_LT, saveGeometry());
	saveADS(WINDOWS_LT, _container->saveState());
}

static QByteArray loadADS(const QString& fname)
{
	QFile f(fname + QString(".dat"));
	if (f.open(QFile::ReadOnly))
	{
		QByteArray ba = f.readAll();
		f.close();
		return ba;
	}
	return QByteArray();
}

static void saveADS(const QString& fname, const QByteArray& ba)
{
	QFile f(fname + QString(".dat"));
	if (f.open(QFile::WriteOnly))
	{
		f.write(ba);
		f.close();
	}
}

static void toggleWindow(int i, bool show)
{
	if (_windows[i].visible == show)
		return;

	if (show)
		_container->showSectionContent(_windows[i].ptr);
	else
		_container->hideSectionContent(_windows[i].ptr);
	_windows[i].visible = show;
}

static void updateMenuButtons()
{
	for(int i = 0; i < WINDOWS_NUM; i++)
	{
		if (_windows[i].ptr)
		{
			_windows[i].visible = _container->isSectionContentVisible(_windows[i].ptr);
			_windows[i].act->setChecked(_windows[i].visible);
		}
	}

	if (editor->IsEngineLoaded() && !engineActions[LOAD]->isChecked())
		engineActions[LOAD]->setChecked(true);

//	if (editor->IsEngineLoaded() != engineActions[RELOAD_CORE_RENDER]->isEnabled())
//		engineActions[RELOAD_CORE_RENDER]->setEnabled(editor->IsEngineLoaded());
}

void MainWindow::on_actionClose_Scene_triggered()
{
	editor->CloseWorld();
}

void MainWindow::on_actionSave_World_triggered()
{
	editor->SaveWorld();
}

void MainWindow::on_actionLoad_World_triggered()
{
	editor->LoadWorld();
}

void MainWindow::on_actionCreate_GameObject_triggered()
{
	editor->CreateGameObject();
}

void MainWindow::on_actionCreate_Light_triggered()
{
	editor->CreateLight();
}

void MainWindow::on_actionCreate_Camera_triggered()
{
	editor->CreateCamera();
}

void MainWindow::on_actionCreate_Model_triggered()
{
	SelectMeshDialog *d = new SelectMeshDialog;
	d->setModal(true);

	auto createMesh = [d](int res)
	{
		if (res == QDialog::Accepted)
		{
			qDebug() << d->mesh;
			editor->CreateModel(d->mesh.toLatin1());
		}
	};

	QObject::connect(d, &QDialog::finished, createMesh);

	d->show();
	d->exec();
	delete d;
}


void MainWindow::switch_button(QAction *action)
{
	ui->actionTranslator->setChecked(ui->actionTranslator == action);
	ui->actionselect->setChecked(ui->actionselect == action);
	ui->actionactionRotator->setChecked(ui->actionactionRotator == action);
}

void MainWindow::on_actionselect_triggered(bool checked)
{
	switch_button(ui->actionselect);
	if (checked) editor->ToggleManipulator(MANIPULATOR::SELECT);
}

void MainWindow::on_actionTranslator_triggered(bool checked)
{
	switch_button(ui->actionTranslator);
	if (checked) editor->ToggleManipulator(MANIPULATOR::TRANSLATE);
}

void MainWindow::on_actionReload_shaders_triggered()
{
	editor->ReloadShaders();
}

void MainWindow::on_actionactionRotator_triggered(bool checked)
{
	switch_button(ui->actionactionRotator);
	if (checked) editor->ToggleManipulator(MANIPULATOR::ROTATE);
}
