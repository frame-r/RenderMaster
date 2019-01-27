#pragma once
#include "Common.h"

class MainWindow;
class FileSystem;
class ResourceManager;
class Console;
class Render;
class SceneManager;

DEFINE_GUID(CLSID_Core,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);

class Core : public ICore
{
	string _pDataDir;
	string _pWorkingDir;
	string _pInstalledDir;

	// subsystems
	unique_ptr<Console> _pConsoleWindow;
	unique_ptr<MainWindow> _pMainWindow;
	unique_ptr<FileSystem>_pfSystem;
	unique_ptr<ResourceManager> _pResMan;
	unique_ptr<ICoreRender>_pCoreRender;
	unique_ptr<Render> _pRender;
	unique_ptr<SceneManager>_pSceneManager;
	unique_ptr<IInput> _pInput;

	CRITICAL_SECTION _cs{};

	vector<IInitCallback*> _initCallbacks;
	vector<std::function<void()>> _updateCallbacks;

	vector<IProfilerCallback*> _profilerCallbacks;
	size_t _records{0};
	std::map<size_t, IProfilerCallback*> _toToRecordFnMap;
	std::map<size_t, uint> _toLocalRecordIdxMap;

	long _lRef{0};

	std::chrono::steady_clock::time_point start;
	int64_t _frame = 0;
	float _dt = 0.0f;
	int _fps = 0;
	int _fpsLazy = 0;

	void _internal_update();
	void _update();
	float _update_fps();
	void _main_loop();
	void static _s_main_loop();
	void _message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void _s_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	void _set_window_caption(int is_paused, int fps);
	void _recreateProfilerRecordsMap();

public:

	Core(const mchar *workingDir, const mchar *installedDir);
	virtual ~Core();

	MainWindow* mainWindow() { return _pMainWindow.get(); }
	Console *consoleWindow() { return _pConsoleWindow.get(); }

	template <typename... Arguments>
	void LogFormatted(const char *pStr, LOG_TYPE type, Arguments ...args)
	{
		static char buf[1000];
		assert(strlen(pStr) < 1000);
		sprintf(buf, pStr, args...);
		Log(buf, type);
	}
	void Log(const char *pStr, LOG_TYPE type = LOG_TYPE::NORMAL);

	void AddUpdateCallback(std::function<void()>&& fn)
	{
		_updateCallbacks.push_back(std::forward<std::function<void()>>(fn));
	}

	void AddProfilerCallback(IProfilerCallback *fn);
	void RemoveProfilerCallback(IProfilerCallback *fn);
	size_t ProfilerRecords();
	string GetProfilerRecord(size_t i);

	float deltaTime() const	{ return _dt; }
	int FPS() const			{ return _fps; }
	int FPSlazy() const		{ return _fpsLazy; }
	int64_t frame() const	{ return _frame; }

	API Init(INIT_FLAGS flags, const mchar *pDataPath, const WindowHandle* externHandle) override;
	API Start() override;
	API Update() override;
	API RenderFrame(const WindowHandle* externHandle, const ICamera *pCamera) override;
	API GetSubSystem(OUT ISubSystem **pSubSystem, SUBSYSTEM_TYPE type) override;
	API GetDataDir(OUT const char **pStr) override;
	API GetWorkingDir(OUT const char **pStr) override;
	API GetInstalledDir(OUT const char **pStr) override;
	API AddInitCallback(IInitCallback *pCallback) override;
	API AddUpdateCallback(IUpdateCallback *pCallback) override;
	API ReleaseEngine() override;

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;
};


class CoreClassFactory : public IClassFactory
{
	long m_lRef;
	long g_lLocks1;

public:

	CoreClassFactory() : m_lRef(0), g_lLocks1(0) {}

	// IUnknown 
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// IClassFactory
	virtual STDMETHODIMP CreateInstance(LPUNKNOWN pUnk, REFIID riid, void** ppv);
	virtual STDMETHODIMP LockServer(BOOL fLock);
};

