#include "Core.h"
#include "ResourceManager.h"
#include "DX11CoreRender.h"
#include "GLCoreRender.h"
#include "Wnd.h"
#include "Console.h"

#include <iostream>


Core *_pCore;

Core::Core() : _pConsole(nullptr), _pWnd(nullptr), _pResMan(nullptr), _pCoreRender(nullptr)
{
	_pCore = this;
	InitializeCriticalSection(&_cs);
}

Core::~Core()
{
	delete _pCoreRender;
	delete _pResMan;
	delete _pWnd;
	if (_pWnd) delete _pWnd;
	if (_pConsole) delete _pConsole;
}

API Core::Init(INIT_FLAGS flags, WinHandle* externHandle)
{
	const bool createWindow = (flags & INIT_FLAGS::IF_WINDOW_FLAG) == INIT_FLAGS::IF_SELF_WINDOW;
	const bool createConsole = (flags & INIT_FLAGS::IF_CONSOLE_FLAG) == INIT_FLAGS::IF_CONSOLE;

	Log("Start initialization engine...");


	_pResMan = new ResourceManager;	

	if (createWindow)
	{
		_pWnd = new Wnd;
		_pWnd->CreateAndShow();
	}

	if (createConsole)
	{
		_pConsole = new Console;
		_pConsole->Init(_pWnd->handle());
	}


	if ((flags & INIT_FLAGS::IF_GRAPHIC_LIBRARY_FLAG) == INIT_FLAGS::IF_DIRECTX11)
		_pCoreRender = new DX11CoreRender;
	else
		_pCoreRender = new GLCoreRender;

	if (createWindow)
		_pCoreRender->Init(_pWnd->handle());
	else
		_pCoreRender->Init(externHandle);

	_pResMan->Init();

	Log("Engine initialized");


	// TODO: main loop move away
	//if (createWindow)
	//{		
	//	_pWnd->StartMainLoop();		
	//}

	return S_OK;
}

API Core::GetSubSystem(ISubSystem *& pSubSystem, SUBSYSTEM_TYPE type)
{
	switch(type)
	{
	case SUBSYSTEM_TYPE::ST_CORE_RENDER: pSubSystem = _pCoreRender; break;
	case SUBSYSTEM_TYPE::ST_RESOURCE_MANAGER: pSubSystem = _pResMan; break;
	default: return S_FALSE;
	}

	return S_OK;
}

API Core::Log(const char *pStr, LOG_TYPE type)
{
	EnterCriticalSection(&_cs);

	if (_pConsole)
		_pConsole->OutputTxt(pStr);

	std::cout << pStr << std::endl;

	_evLog.Fire(pStr, type);

	LeaveCriticalSection(&_cs);

	return S_OK;
}

API Core::CloseEngine()
{
	Log("Core::CloseEngine()");

	_pResMan->FreeAllResources();

	_pCoreRender->Free();

#ifdef _DEBUG
	system("pause");
#endif

	if (_pConsole)
		_pConsole->Destroy();

	if (_pWnd)
		_pWnd->Destroy();

	return S_OK;
}

API Core::GetLogPrintedEv(ILogEvent*& pEvent)
{
	pEvent = &_evLog;
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
