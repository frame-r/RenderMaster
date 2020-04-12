#pragma once
#include "common.h"
#include "console.h"

extern Core *_core;


class Core final : IProfilerCallback
{
	// Pathes without end backslashes
	std::string rootPath_;
	std::string workingPath_;
	std::string dataPath_;

	// Subsystems
	ResourceManager *resMan{nullptr};
	ICoreRender *coreRender{nullptr};
	FileSystem *fs{nullptr};
	Render *render{nullptr};
	MainWindow *window{nullptr};
	Console *console{nullptr};
	Input *input{nullptr};
	MaterialManager *matManager{nullptr};

	Signal<float> onUpdate;
	Signal<> onInit;

	int64_t _frame{0};
	float _dt{0.0f};
	int _fps{0};
	int _fpsLazy{0};
	bool _profiler{false};

	Camera *camera{nullptr};

	int MSAASamples{1};
	int VSync{1};

	std::vector<IProfilerCallback*> profilerCallbacks;

	std::mutex logMtx;
	char logBuffer__[5000];

	void freeCoreRender();
	bool initCoreRender(WindowHandle *handle);
	void engineUpdate();
	void mainLoop();
	void setWindowCaption(int is_paused, int fps);
	void messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);

	static void sMessageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void sMainLoop();

public:
	// IProfilerCallback
	uint getNumLines() override;
	std::string getString(uint i) override;

public:
	// Internal API
	Core();
	~Core();
	int ProfilerCallbacks() const { return (int)profilerCallbacks.size(); }
	IProfilerCallback *getCallback(int i) const { return profilerCallbacks[i]; }
	float deltaTime() { return _dt; }
	int64_t frame() { return _frame; }

	template<class T, typename... Arguments>
	void _Log(LOG_TYPE type, T a, Arguments ...args)
	{
		std::lock_guard<std::mutex> guard(logMtx);

		if (strlen(a) > 4999) abort();
		sprintf(logBuffer__, a, args...);
		if (_core->GetConsole())
			_core->GetConsole()->Log(logBuffer__, type);
	}

public:

	auto DLLEXPORT Init(const char* rootPath, const WindowHandle* externHandle, INIT_FLAGS flags = INIT_FLAGS::NONE) -> bool;
	auto DLLEXPORT Free() -> void;
	auto DLLEXPORT Start(Camera *cam) -> void;
	auto DLLEXPORT ManualUpdate() -> void;
	auto DLLEXPORT ManualRenderFrame(const WindowHandle* externHandle, const Engine::CameraData& camera, Model** wireframeModels, int modelsNum) -> void;

	auto DLLEXPORT GetRootPath() -> const std::string& { return rootPath_; }
	auto DLLEXPORT GetDataPath() -> const std::string& { return dataPath_; }
	auto DLLEXPORT GetWorkingPath() -> const std::string& { return workingPath_; }

	auto DLLEXPORT GetResourceManager() -> ResourceManager* { return resMan; }
	auto DLLEXPORT GetMaterialManager() -> MaterialManager* { return matManager; }
	auto DLLEXPORT GetRender() -> Render* { return render; }
	auto DLLEXPORT GetCoreRender() -> ICoreRender* { return coreRender; }
	auto DLLEXPORT GetFilesystem() -> FileSystem* { return fs; }
	auto DLLEXPORT GeWindow() -> MainWindow* { return window; }
	auto DLLEXPORT GetConsole() -> Console* { return console; }
	auto DLLEXPORT GetInput() -> Input* { return input; }

	auto DLLEXPORT AddProfilerCallback(IProfilerCallback *c) -> void;
	auto DLLEXPORT RemoveProfilerCallback(IProfilerCallback *c) -> void;
	auto DLLEXPORT SetProfiler(bool value) -> void {_profiler = value; }
	auto DLLEXPORT IsProfiler() -> bool { return _profiler; }
};

DLLEXPORT Core* GetCore();
DLLEXPORT void ReleaseCore(Core* core);

template<typename... Arguments>
void Log(Arguments ...args)
{
	_core->_Log(LOG_TYPE::NORMAL, args...);
}
template<typename... Arguments>
void LogWarning(Arguments ...args)
{
	_core->_Log(LOG_TYPE::WARNING, args...);
}
template<typename... Arguments>
void LogCritical(Arguments ...args)
{
	_core->_Log(LOG_TYPE::CRITICAL, args...);
}
template<typename... Arguments>
void LogFatal(Arguments ...args)
{
	_core->_Log(LOG_TYPE::FATAL, args...);
}



