#pragma once
#include "Common.h"
#include <chrono>

class Wnd;
class FileSystem;
class ResourceManager;
class Console;
class EventLog;
class Render;
class SceneManager;

DEFINE_GUID(CLSID_Core,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);

class Core : public ICore
{
	char *_pDataDir{nullptr};
	char *_pWorkingDir{nullptr};
	char *_pInstalledDir{nullptr};	

	Console *_pConsole{nullptr};
	Wnd *_pWnd{nullptr};
	FileSystem *_pfSystem{nullptr};
	ResourceManager *_pResMan{nullptr};
	ICoreRender *_pCoreRender{nullptr};
	Render *_pRender{nullptr};
	SceneManager *_pSceneManager{nullptr};
	IInput *_pInput{nullptr};

	EventLog *_evLog{nullptr};

	CRITICAL_SECTION _cs{};

	std::vector<IInitCallback *> _init_callbacks;
	std::vector<std::function<void()>> _update_callbacks;

	long _lRef{0};

	std::chrono::steady_clock::time_point start;

	float update_fps();
	std::string _getFullLogPath();
	void _main_loop();
	void static _s_main_loop();
	void _message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void _s_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);

public:

	Core(const char *workingDir, const char *installedDir);
	~Core();

	template <typename... Arguments>
	void LogFormatted(const char *pStr, LOG_TYPE type, Arguments ...args)
	{
		static char buf[1000];
		assert(strlen(pStr) < 1000);
		sprintf_s(buf, pStr, args...);
		Log(buf, type);
	}
	Wnd* MainWindow() { return _pWnd; }
	void AddUpdateCallback(std::function<void()>&& calback) { _update_callbacks.push_back(std::forward<std::function<void()>>(calback)); }

	API Init(INIT_FLAGS flags, const char *pDataPath, const WinHandle* handle) override;
	API Start() override;
	API RenderFrame() override;
	API GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type) override;
	API GetDataDir(const char *&pStr) override;
	API GetWorkingDir(const char *&pStr) override;
	API GetInstalledDir(const char *&pStr) override;
	API Log(const char *pText, LOG_TYPE type = LOG_TYPE::NORMAL) override;
	API AddInitCallback(IInitCallback *pCallback) override;
	API AddUpdateCallback(IUpdateCallback *pCallback) override;
	API CloseEngine() override;
	API GetLogPrintedEv(ILogEvent *&pEvent) override;

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

