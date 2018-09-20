#include "Core.h"
#include "Filesystem.h"
#include "ResourceManager.h"
#include "DX11CoreRender.h"
#include "GLCoreRender.h"
#include "Wnd.h"
#include "Console.h"
#include "Events.h"
#include "Render.h"
#include "SceneManager.h"
#include "Input.h"

#include <iostream>
#include <fstream>

Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


Core::Core(const char *pWorkingDir, const char *pInstalledDir)
{
	_pCore = this;
	
	InitializeCriticalSection(&_cs);

	_pWorkingDir = new char[strlen(pWorkingDir) + 1];
	strcpy(_pWorkingDir, pWorkingDir);

	_pInstalledDir = new char[strlen(pInstalledDir) + 1];
	strcpy(_pInstalledDir, pInstalledDir);	
}

Core::~Core()
{
	delete _pSceneManager;
	delete _pRender;
	delete _pCoreRender;
	delete _pResMan;
	if (_pMainWindow) delete _pMainWindow;
	if (_pConsole) delete _pConsole;
	delete _pInput;
	delete _pfSystem;
	delete _pDataDir;
	delete _pWorkingDir;
	delete _pInstalledDir;
}

API Core::Init(INIT_FLAGS flags, const char *pDataPath, const WinHandle* externHandle)
{
	const bool createWindow = (flags & INIT_FLAGS::WINDOW_FLAG) != INIT_FLAGS::EXTERN_WINDOW;
	const bool createConsole = (flags & INIT_FLAGS::CREATE_CONSOLE_FLAG) == INIT_FLAGS::CREATE_CONSOLE;

	std::string absoluteDataPath = pDataPath;
	if (is_relative(pDataPath))
	{
		absoluteDataPath = make_absolute(pDataPath, _pInstalledDir);
	}

	int exist = 0;
	_pfSystem->DirectoryExist(absoluteDataPath.c_str(), &exist);
	if (!exist)
	{
		std::cout << "Core::Init(): directory" << absoluteDataPath.c_str() << " doesn't exist\n" << std::endl;
		return E_ABORT;
	}

	_pDataDir = new char[absoluteDataPath.size() + 1];
	strcpy(_pDataDir, absoluteDataPath.c_str());

	std::ofstream log(_getFullLogPath());
	log.close();

	if (createConsole)
	{
		_pConsole = new Console;
		_pConsole->Init(nullptr);
	}

	Log("Start initialization engine...");
	LogFormatted("Working directory:    %s", LOG_TYPE::NORMAL, _pWorkingDir);
	LogFormatted("Installed directory:  %s", LOG_TYPE::NORMAL, _pInstalledDir);
	LogFormatted("Data directory:       %s", LOG_TYPE::NORMAL, _pDataDir);	

	if (createWindow)
	{
		_pMainWindow = new Wnd(_s_main_loop);
		_pMainWindow->AddMessageCallback(_s_message_callback);
		_pMainWindow->CreateAndShow();
	}

	_pfSystem = new FileSystem(_pDataDir);

	_pResMan = new ResourceManager;	

	_pInput = new Input;

	if ((flags & INIT_FLAGS::GRAPHIC_LIBRARY_FLAG) == INIT_FLAGS::DIRECTX11)
		_pCoreRender = new DX11CoreRender;
	else
		_pCoreRender = new GLCoreRender;

	if (createWindow)
		_pCoreRender->Init(_pMainWindow->handle());
	else
		_pCoreRender->Init(externHandle);

	_pResMan->Init();

	_pSceneManager = new SceneManager();
	_pSceneManager->Init();

	_pRender = new Render(_pCoreRender);
	_pRender->Init();

	if (createWindow)
		_pMainWindow->Show();

	Log("Engine initialized");

	return S_OK;
}

API Core::Start()
{
	for (IInitCallback *callback : _init_callbacks)
		callback->Init();

	//timer_fps.Start();

	if (_pMainWindow)
	{
		uint w, h;
		_pMainWindow->GetDimension(w, h);
		_pCoreRender->SetViewport(w, h);
		_pMainWindow->StartMainLoop();
	}

	return S_OK;
}

API Core::RenderFrame(const WinHandle* extern_handle, const ICamera *pCamera)
{
	_pCoreRender->MakeCurrent(extern_handle);

#ifdef WIN32
	RECT r;
	GetWindowRect(*extern_handle, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
#else
	assert(false); // not impl
#endif // WIN32
	
	_pCoreRender->SetViewport(w, h);
	_pRender->RenderFrame(pCamera);

	return S_OK;
}

API Core::GetSubSystem(OUT ISubSystem **pSubSystem, SUBSYSTEM_TYPE type)
{
	switch(type)
	{
		case SUBSYSTEM_TYPE::CORE_RENDER: *pSubSystem = _pCoreRender; break;
		case SUBSYSTEM_TYPE::RESOURCE_MANAGER: *pSubSystem = _pResMan; break;
		case SUBSYSTEM_TYPE::FILESYSTEM: *pSubSystem = _pfSystem; break;
		case SUBSYSTEM_TYPE::INPUT: *pSubSystem = _pInput; break;
		case SUBSYSTEM_TYPE::SCENE_MANAGER: *pSubSystem = _pSceneManager; break;
		case SUBSYSTEM_TYPE::RENDER: *pSubSystem = _pRender; break;
		default:
			LOG_WARNING("Core::GetSubSystem() unknown subsystem");
			return S_FALSE;
	}

	return S_OK;
}

void Core::_main_loop()
{
	update_fps();

	for (auto &callback : _update_callbacks)
		callback();

	ICamera *cam;
	_pSceneManager->GetDefaultCamera(&cam);

	_pRender->RenderFrame(cam);
	_pCoreRender->SwapBuffers();
}

void Core::_s_main_loop()
{
	_pCore->_main_loop();
}

void Core::_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	switch (type)
	{
	case WINDOW_MESSAGE::SIZE:
		if (_pCoreRender)
			_pCoreRender->SetViewport(param1, param2);
		//LogFormatted("Window size changed: x=%i y=%i", LOG_TYPE::NORMAL, param1, param2);
		break;

	case WINDOW_MESSAGE::WINDOW_UNMINIMIZED:
		if (_pConsole)
			_pConsole->Show();
		break;

	case WINDOW_MESSAGE::WINDOW_MINIMIZED:
		if (_pConsole)
			_pConsole->Hide();
		break;

	case WINDOW_MESSAGE::WINDOW_REDRAW:
		ICamera * cam;
		_pSceneManager->GetDefaultCamera(&cam);
		_pRender->RenderFrame(cam);
		_pCoreRender->SwapBuffers();
		break;

	case WINDOW_MESSAGE::WINDOW_CLOSE:
		if (_pConsole)
			_pConsole->BringToFront();
		break;

	default:
		break;
	}
}

void Core::_s_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	_pCore->_message_callback(type, param1, param2, pData);
}

std::string Core::_getFullLogPath()
{
	return std::string(_pDataDir) + "\\log.txt";
}

API Core::GetDataDir(OUT char **pStr)
{
	*pStr = _pDataDir;
	return S_OK;
}

API Core::GetWorkingDir(OUT char **pStr)
{
	*pStr = _pWorkingDir;
	return S_OK;
}

API Core::GetInstalledDir(OUT char **pStr)
{
	*pStr = _pInstalledDir;
	return S_OK;
}

API Core::Log(const char *pStr, LOG_TYPE type)
{
	EnterCriticalSection(&_cs);

	if (_pConsole)
		_pConsole->OutputTxt(pStr);

	std::ofstream log(_getFullLogPath(), std::ios::out | std::ios::app);
	log << pStr << std::endl;
	log.close();

	std::cout << pStr << std::endl;

	_evLog->Fire(pStr, type);

	LeaveCriticalSection(&_cs);

	return S_OK;
}

API Core::AddInitCallback(IInitCallback* pCallback)
{
	_init_callbacks.push_back(pCallback);
	return S_OK;
}

API Core::AddUpdateCallback(IUpdateCallback* pCallback)
{
	_update_callbacks.push_back(std::bind(&IUpdateCallback::Update, pCallback));
	return S_OK;
}

API Core::CloseEngine()
{
	Log("Start closing Engine...");

	if (_pMainWindow)
		_pMainWindow->Destroy();
	
	_pSceneManager->Free();

	_pRender->Free();

	_pResMan->Free();

	_pCoreRender->Free();

	Log("Engine Closed");

	if (_pConsole)
		_pConsole->Destroy();

	return S_OK;
}

API Core::GetLogPrintedEv(OUT ILogEvent **pEvent)
{
	*pEvent = _evLog.get();
	return S_OK;
}

float Core::update_fps()
{
	static const float upd_interv = 0.3f;
	static float accum = 0.0f;

	std::chrono::duration<float> _durationSec = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
	float sec = _durationSec.count();

	accum += sec;

	if (accum > upd_interv)
	{
		accum = 0.0f;
		int fps = static_cast<int>(1.0f / sec);
		std::string fps_str = std::to_string(fps);
		std::wstring fps_strw = std::wstring(L"Test [") + std::wstring(fps_str.begin(), fps_str.end()) + std::wstring(L"]");

		_pMainWindow->SetCaption(fps_strw.c_str());
	}

	start = std::chrono::steady_clock::now();

	return sec;
}

HRESULT Core::QueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_Core || riid == IID_IUnknown)
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG Core::AddRef()
{
	InterlockedIncrement(&_lRef);
	return 0;
}

ULONG Core::Release()
{
	InterlockedDecrement(&_lRef);
	if (_lRef == 0)
	{
		delete this;
		return 0;
	}
	else
		return _lRef;
}

HRESULT CoreClassFactory::QueryInterface(REFIID riid, void ** ppv)
{
	*ppv = 0;

	if (riid == IID_IUnknown || riid == IID_IClassFactory)
		*ppv = this;

	if (*ppv)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG CoreClassFactory::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CoreClassFactory::Release()
{
	if (InterlockedDecrement(&m_lRef) == 0)
	{
		delete this;
		return 0;
	}

	return m_lRef;
}

STDMETHODIMP CoreClassFactory::CreateInstance(LPUNKNOWN pUnk, REFIID riid, void ** ppvObj)
{
	Core* pCore;
	HRESULT hr;
	*ppvObj = 0;
	
	// Recieve working directory from Registry
	
	auto get_registry_value = [](TCHAR *key) -> std::string
	{
		std::wstring BaseKey(TEXT("SOFTWARE\\Classes\\CLSID\\{A889F560-58E4-11d0-A68A-0000837E3100}\\InProcServer32\\"));

		DWORD buffer_size = 0;

		RegGetValue(HKEY_LOCAL_MACHINE, BaseKey.c_str(), key, RRF_RT_REG_SZ, 0, nullptr, // pvData == nullptr ? Request buffer size
			&buffer_size);

		const DWORD buffer_length = buffer_size / sizeof(WCHAR); // length in WCHAR's
		std::unique_ptr<WCHAR[]> text_buffer (new WCHAR[buffer_length]);

		RegGetValue(HKEY_LOCAL_MACHINE, BaseKey.c_str(), key, RRF_RT_REG_SZ, 0, text_buffer.get(), &buffer_size);

		return ConvertFromUtf16ToUtf8(text_buffer.get());
	};
	
	std::string workingDir = get_registry_value(TEXT("WorkingDir"));
	std::string installedDir = get_registry_value(TEXT("InstalledDir"));

	pCore = new Core(workingDir.c_str(), installedDir.c_str());


	if (pCore == 0)
		return E_OUTOFMEMORY;

	hr = pCore->QueryInterface(riid, ppvObj);

	if (FAILED(hr))
		delete pCore;

	return hr;
}

STDMETHODIMP CoreClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		InterlockedIncrement(&g_lLocks1);
	else
		InterlockedDecrement(&g_lLocks1);

	return S_OK;
}

