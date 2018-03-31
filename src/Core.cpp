#include "Core.h"
#include "Filesystem.h"
#include "ResourceManager.h"
#include "DX11CoreRender.h"
#include "GLCoreRender.h"
#include "Wnd.h"
#include "Console.h"
#include "Events.h"

#include <iostream>
#include <fstream>

Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

std::string Core::_getFullLogPath()
{
	return std::string(_pDataPath) + "\\log.txt";
}

void Core::_main_loop()
{
	for (IUpdateCallback *callback : _update_callbacks)
		callback->Update();

	_pCoreRender->Clear();
	_pCoreRender->SwapBuffers();
}

void Core::_s_main_loop()
{
	_pCore->_main_loop();
}

Core::Core(const char *workingDir)
{
	_pWorkingPath = new char[strlen(workingDir) + 1];
	strcpy(_pWorkingPath, workingDir);

	_pCore = this;

	InitializeCriticalSection(&_cs);
}

Core::~Core()
{
	delete _pCoreRender;
	delete _pResMan;
	if (_pWnd) delete _pWnd;
	if (_pConsole) delete _pConsole;
	delete _pfSystem;
	delete _evLog;
	delete _pDataPath;
	delete[] _pWorkingPath;
}

API Core::Init(INIT_FLAGS flags, const char *pDataPath, WinHandle* externHandle)
{
	const bool createWindow = (flags & INIT_FLAGS::WINDOW_FLAG) != INIT_FLAGS::EXTERN_WINDOW;
	const bool createConsole = (flags & INIT_FLAGS::CREATE_CONSOLE_FLAG) == INIT_FLAGS::CREATE_CONSOLE;

	std::string absoluteDataPath = pDataPath;
	if (is_relative(pDataPath))
	{
		absoluteDataPath = make_absolute(pDataPath, _pWorkingPath);
	}
	_pDataPath = new char[absoluteDataPath.size() + 1];
	strcpy(_pDataPath, absoluteDataPath.c_str());

	std::ofstream log(_getFullLogPath());
	log.close();

	_evLog = new EventLog;

	if (createConsole)
	{
		_pConsole = new Console;
		_pConsole->Init(nullptr);
	}

	Log("Start initialization engine...");
	LogFormatted("Working path: %s", LOG_TYPE::NORMAL, _pWorkingPath);
	LogFormatted("Data path: %s", LOG_TYPE::NORMAL, _pDataPath);

	_pfSystem = new FileSystem(pDataPath);

	_pResMan = new ResourceManager;	

	if (createWindow)
	{
		_pWnd = new Wnd(_s_main_loop);
		_pWnd->CreateAndShow();
	}

	if ((flags & INIT_FLAGS::GRAPHIC_LIBRARY_FLAG) == INIT_FLAGS::DIRECTX11)
		_pCoreRender = new DX11CoreRender;
	else
		_pCoreRender = new GLCoreRender;

	if (createWindow)
		_pCoreRender->Init(_pWnd->handle());
	else
		_pCoreRender->Init(externHandle);

	_pResMan->Init();

	Log("Engine initialized");

	return S_OK;
}

API Core::GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type)
{
	switch(type)
	{
		case SUBSYSTEM_TYPE::CORE_RENDER: pSubSystem = _pCoreRender; break;
		case SUBSYSTEM_TYPE::RESOURCE_MANAGER: pSubSystem = _pResMan; break;
		case SUBSYSTEM_TYPE::FILESYSTEM: pSubSystem = _pfSystem; break;
		default:
			LOG_WARNING("Core::GetSubSystem() unknown subsystem");
			return S_FALSE;
	}

	return S_OK;
}

API Core::GetDataPath(const char *&pStr)
{
	pStr = _pDataPath;
	return S_OK;
}

API Core::GetWorkingPath(const char *& pStr)
{
	pStr = _pWorkingPath;
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
	_update_callbacks.push_back(pCallback);
	return S_OK;
}

API Core::Start()
{
	for (IInitCallback *callback : _init_callbacks)
		callback->Init();

	if (_pWnd)
		_pWnd->StartMainLoop();

	return S_OK;
}

API Core::CloseEngine()
{
	Log("Core::CloseEngine()");

	_pResMan->FreeAllResources();

	_pCoreRender->Free();

	if (_pConsole)
		_pConsole->Destroy();

	if (_pWnd)
		_pWnd->Destroy();

	return S_OK;
}

API Core::GetLogPrintedEv(ILogEvent*& pEvent)
{
	pEvent = _evLog;
	return S_OK;
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
	
	// Recieve working path from Registry

	std::wstring BaseKey(TEXT("SOFTWARE\\Classes\\CLSID\\{A889F560-58E4-11d0-A68A-0000837E3100}\\InProcServer32\\"));
	std::wstring Value(TEXT("InstalledDir"));

	DWORD buffer_size = 0;

	RegGetValue(HKEY_LOCAL_MACHINE, BaseKey.c_str(), Value.c_str(), RRF_RT_REG_SZ, 0, nullptr, // pvData == nullptr ? Request buffer size
		&buffer_size);

	const DWORD buffer_length = buffer_size / sizeof(WCHAR); // length in WCHAR's
	WCHAR* const text_buffer = new WCHAR[buffer_length];

	RegGetValue(HKEY_LOCAL_MACHINE, BaseKey.c_str(), Value.c_str(), RRF_RT_REG_SZ, 0, text_buffer, &buffer_size);

	std::string working_dir = ConvertFromUtf16ToUtf8(text_buffer);

	pCore = new Core(working_dir.c_str());

	delete[] text_buffer;

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

