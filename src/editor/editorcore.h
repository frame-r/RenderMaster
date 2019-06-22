#ifndef EDITORGLOBAL_H
#define EDITORGLOBAL_H
#include "mainwindow.h"
#include "manipulators/imanipulator.h"

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QSet>

#include <chrono>

#include "core.h"
#include "resource_manager.h"

class EditorCore;
class TreeNode;
class QApplication;

extern EditorCore *editor;


enum class MANIPULATOR
{
	SELECT,
	TRANSLATE,
	ROTATE,
	SCALE
};


class EditorCore : public QObject
{
	Q_OBJECT

	QApplication& app_;
	bool isActive{true};
	bool preventFocusOnWorldLoad{false};

	QTimer *timer{nullptr};
	std::chrono::steady_clock::time_point start;

	TreeNode *rootNode{nullptr};
	QHash<GameObject*, TreeNode*> obj_to_treenode;

	QSet<GameObject*> selectedObjects;

	MANIPULATOR maipulatorType_ = MANIPULATOR::SELECT;

	void onEngineInited();
	void onEngineFree();
	static void onEngineObejctAdded(GameObject *obj);
	static void onEngineObejctDestroyed(GameObject *obj);

public:
	EditorCore(QApplication& app);
	~EditorCore();

	Core *core{nullptr};
	ResourceManager *resMan{nullptr};

	MainWindow *window{nullptr};

	auto Init() -> void;
	auto Free() -> void;

	//----Engine-----

	auto LoadEngine() -> void;
	auto UnloadEngine() -> void;
	auto IsEngineLoaded() -> bool { return core != nullptr; }
	//auto ReloadCoreRender() -> void;
	auto ReloadShaders() -> void;


	//----Objects-----

	// Nodes
	auto RootNode() -> TreeNode* { return rootNode; }
	auto TreNodeForObject(GameObject *o) { return obj_to_treenode[o]; }

	// Game objects
	auto CreateGameObject() -> void;
	auto CreateModel(const char *path) -> void;
	auto CreateLight() -> void;
	auto CreateCamera() -> void;

	auto CloneSelectedGameObject() -> void;
	auto DestroyGameObject(GameObject* obj) -> void;
	auto InsertRootGameObject(int row, GameObject* obj) -> void;
	auto RemoveRootGameObject(GameObject* obj) -> void;

	auto SaveWorld() -> void;
	auto CloseWorld() -> void;
	auto LoadWorld() -> void;

	// Selection objects
	auto SelectObjects(const QSet<GameObject*>& objects) -> void;
	auto FirstSelectedObjects() -> GameObject*;
	auto NumSelectedObjects() -> int { return selectedObjects.size(); }
	auto SelectionTransform() -> mat4;
	auto Focus() -> void;

	//---Manipulators---
	void ToggleManipulator(MANIPULATOR type);
	std::unique_ptr<IManupulator> currentManipulator;

signals:
	void OnUpdate(float dt);
	void OnRender();
	void OnEngineInstantiated(Core* c);
	void OnEngineInit(Core* c);
	void OnEngineFree(Core* c);
	void OnObjectAdded(TreeNode *obj);
	void OnObjectRemoved(TreeNode *obj);
	void OnSelectionChanged(QSet<GameObject*>& objects);
	void OnFocusOnSelected(const vec3& ceneter);

private slots:
	void OnTimer();
	void OnAppStateChanged(Qt::ApplicationState state);
};

extern EditorCore *editor;


#endif // EDITORGLOBAL_H
