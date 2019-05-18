#include "editorcore.h"
#include <QDebug>
#include "core.h"
#include "camera.h"
#include "render.h"
#include "gameobject.h"
#include <QVariant>
#include <QApplication>
#include "treenode.h"
#include "manipulators/manipulatortranslator.h"

#define upd_interv 1.0f
#define timer_interval 16

#define CHECK_ENGINE \
	if (!IsEngineLoaded()) \
	{ \
		qDebug() << "EditorCore::CreateGameObject(): engine is not loaded"; \
		return; \
	}


EditorCore *editor;

EditorCore::EditorCore(QApplication& app) : app_(app)
{
	editor = this;

	//QVariant rootData = "rootNode";
	rootNode = new TreeNode(nullptr, nullptr);
	obj_to_treenode[nullptr] = rootNode;

	timer = new QTimer(this);
	window = new MainWindow;

	connect(&app_, &QGuiApplication::applicationStateChanged, this, &EditorCore::OnAppStateChanged);
}

void EditorCore::onEngineInited()
{
	resMan->AddCallbackOnObjAdded(onEngineObejctAdded);
	resMan->AddCallbackOnDestroy(onEngineObejctDestroyed);

	//dbg
	resMan->LoadWorld();
}

void EditorCore::onEngineFree()
{
	currentManipulator = nullptr;
	//resMan->RemoveCallbackOnObjAdded(onEngineObejctAdded);
	//resMan->RemoveCallbackOnObjDestroyed(onEngineObejctDestroyed);
}

EditorCore::~EditorCore()
{
	delete rootNode;
	editor = nullptr;
}

void EditorCore::Init()
{
	connect(timer, SIGNAL(timeout()), this, SLOT(OnTimer()));

	window->show();

	// load engine at start
	LoadEngine();
}

void EditorCore::Free()
{
	UnloadEngine();

	delete window;
	delete timer;
}

void EditorCore::LoadEngine()
{
	if (IsEngineLoaded())
		return;

	core = GetCore();

	if (!core)
		ReleaseCore(core);
	else
	{
		emit OnEngineInstantiated(core);

		HWND h = (HWND)window->getWindowHandle();

		core->Init("../../", &h, INIT_FLAGS::NONE);
		resMan = core->GetResourceManager();

		onEngineInited();
		emit OnEngineInit(core);

		// dbg
		//resMan->CreateCamera();

		start = std::chrono::steady_clock::now();
		timer->start(timer_interval);
	}
}

void EditorCore::UnloadEngine()
{
	CHECK_ENGINE

	timer->stop();

	if (core)
	{
		emit OnEngineFree(core);
		onEngineFree();

		core->Free();

		ReleaseCore(core);
		core = nullptr;
		resMan = nullptr;
	}
}

void EditorCore::ReloadShaders()
{
	CHECK_ENGINE
	core->GetRender()->ReloadShaders();
}

//void EditorCore::ReloadCoreRender()
//{
//	CHECK_ENGINE
//	core->ReloadCoreRender();
//}

void EditorCore::CreateGameObject()
{
	CHECK_ENGINE
	resMan->CreateGameObject(); // must be handeled by EditorCore::onObejctAdded(GameObject *obj);
}

void EditorCore::CreateModel(const char *path)
{
	CHECK_ENGINE
	resMan->CreateModel(path); // must be handeled by EditorCore::onObejctAdded(GameObject *obj);
}

void EditorCore::CreateLight()
{
	CHECK_ENGINE
	resMan->CreateLight(); // must be handeled by EditorCore::onObejctAdded(GameObject *obj);
}

void EditorCore::CreateCamera()
{
	CHECK_ENGINE
	resMan->CreateCamera(); // must be handeled by EditorCore::onObejctAdded(GameObject *obj);
}

void EditorCore::CloneSelectedGameObject()
{
	CHECK_ENGINE

	if (!selectedObjects.size())
		return;

	// copy because selectedObject can change while cloning
	QSet<GameObject*> selected = selectedObjects;

	for(GameObject *o : selected)
	{
		resMan->CloneObject(o); // must be handeled by EditorCore::onObejctAdded(GameObject *obj);
	}
}

void EditorCore::DestroyGameObject(GameObject *obj)
{
	CHECK_ENGINE

	resMan->DestroyObject(obj);

	//auto it = obj_to_treenode.find(obj);
	//if (it == obj_to_treenode.end())
	//{
	//	qDebug() << "it should not be happened";
	//	return;
	//}
	//
	//TreeNode *node = *it;
	//if (!node || node == rootNode)
	//	return;
	//
	//if (obj->GetParent() == nullptr)
	//	resMan->RemoveObject(obj);
	//else
	//	obj->GetParent()->RemoveChild(obj);
	//
	//emit OnObjectRemoved(node);
	//
	////TreeNode *parent = node->parentNode();
	////parent->removeChild(node->row());
	//
	//delete obj;
	//delete node;

	//obj_to_treenode.erase(it);
	//qDebug() << "obj_to_treenode.size()=" << editor->obj_to_treenode.size();
}

void EditorCore::onEngineObejctAdded(GameObject *obj)
{
	auto it = editor->obj_to_treenode.find(obj);
	if (it != editor->obj_to_treenode.end())
	{
		qDebug() << "it should not be happened";
		return;
	}

	// dbg
	static int counter;
	counter++;
	obj->SetName(QString("GameObject %1").arg(counter).toLatin1());
	//obj->SetWorldPosition({1, 0, 0});

	GameObject *parent = obj->GetParent();
	TreeNode *parentNode = editor->obj_to_treenode[parent];
	TreeNode *node = new TreeNode(obj, parentNode);
	parentNode->appendChild(node);
	editor->obj_to_treenode[obj] = node;

	emit editor->OnObjectAdded(node);

	//qDebug() << "obj_to_treenode.size()=" << editor->obj_to_treenode.size() - 1;
}

void EditorCore::onEngineObejctDestroyed(GameObject *obj)
{
	auto it = editor->obj_to_treenode.find(obj);
	if (it == editor->obj_to_treenode.end())
	{
		qDebug() << "it should not be happened";
		return;
	}

	TreeNode *node = *it;
	if (!node || node == editor->rootNode)
		return;

	emit editor->OnObjectRemoved(node);

	delete node;

	editor->obj_to_treenode.erase(it);
	//qDebug() << "obj_to_treenode.size()=" << editor->obj_to_treenode.size() - 1;
}


void EditorCore::InsertRootGameObject(int row, GameObject *obj)
{
	CHECK_ENGINE
	resMan->InsertObject(row, obj);
}

void EditorCore::RemoveRootGameObject(GameObject *obj)
{
	CHECK_ENGINE
	resMan->RemoveObject(obj);
}

void EditorCore::SaveWorld()
{
	resMan->SaveWorld();
}

void EditorCore::CloseWorld()
{
	resMan->CloseWorld();
}

void EditorCore::LoadWorld()
{
	resMan->LoadWorld();
}

void EditorCore::SelectObjects(const QSet<GameObject *> &objects)
{
	if (objects != selectedObjects)
	{
		selectedObjects = objects;
		emit OnSelectionChanged(selectedObjects);
	}
}

GameObject* EditorCore::FirstSelectedObjects()
{
	assert(NumSelectedObjects() > 0);
	return *selectedObjects.begin();
}

mat4 EditorCore::SelectionTransform()
{
	assert(NumSelectedObjects() == 1);
	GameObject * g = *selectedObjects.begin();

	mat4 tr = g->GetWorldTransform();

	vec3 t, s;
	quat r;
	decompositeTransform(tr, t, r, s);
	compositeTransform(tr, t, r, vec3(1,1,1));

	return tr;
}

void EditorCore::Focus()
{
	if (NumSelectedObjects() == 0)
		return;

	if (NumSelectedObjects() != 1) // TODO: multiple objects
		return;

	GameObject * g = *selectedObjects.begin();
	emit OnFocusOnSelected(g->GetWorldPosition());
}

void EditorCore::ToggleManipulator(MANIPULATOR type)
{
	if (maipulatorType_ == type)
		return;

	maipulatorType_ = type;

	switch (type)
	{
		case MANIPULATOR::SELECT: currentManipulator = nullptr; break;
		case MANIPULATOR::TRANSLATE: currentManipulator = std::unique_ptr<IManupulator>(new ManipulatorTranslator()); break;
		default: currentManipulator = nullptr; break;
	}
}

void EditorCore::OnTimer()
{
	if (!IsEngineLoaded() || !isActive)
		return;

	//static float accum = 0.0f;

	std::chrono::duration<float> _durationSec = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
	float dt = _durationSec.count();

	//accum += dt;

	//if (accum > upd_interv)
	//{
	//	accum = 0.0f;
	//	int fps = static_cast<int>(1.0f / dt);
	//	std::string fps_str = std::to_string(fps);
	//	std::wstring fps_strw = std::wstring(L"Test [") + std::wstring(fps_str.begin(), fps_str.end()) + std::wstring(L"]");
	//	static char buf[40];
	//	//sprintf(buf, "FPS = %i", fps);
	//   // pCore->Log(buf, RENDER_MASTER::LOG_TYPE::NORMAL);
	//}

	start = std::chrono::steady_clock::now();

	core->ManualUpdate(); // engine update once

	emit OnUpdate(dt); // all editor-related logic updates
	emit OnRender();
}

void EditorCore::OnAppStateChanged(Qt::ApplicationState state)
{
	//if (state == Qt::ApplicationState::ApplicationSuspended)
	//	qDebug() << "Qt::ApplicationState::ApplicationSuspended" << state;
	if (state == Qt::ApplicationState::ApplicationActive)
	{
		timer->start(timer_interval);
		start = std::chrono::steady_clock::now();
		isActive = true;
	}
	else if (state == Qt::ApplicationState::ApplicationInactive)
	{
		timer->stop();
		isActive = false;
	}
}
