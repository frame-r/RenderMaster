#include "Core.h"
#include "ResourceManager.h"
#include "DX11CoreRender.h"
#include "GLCoreRender.h"
#include "Wnd.h"

#include <iostream>


Core *_pCore;

Core::Core() : _pWnd(nullptr), _pResMan(nullptr), _pCoreRender(nullptr)
{
	_pCore = this;
}

Core::~Core()
{
	delete _pCoreRender;
	delete _pResMan;
	delete _pWnd;
}

API Core::Init(INIT_FLAGS flags, WinHandle& handle)
{
	WinHandle h = handle;

	Log("Start initialization engine...");

	_pResMan = new ResourceManager;	

	if ((flags & INIT_FLAGS::IF_WINDOW_FLAG) == INIT_FLAGS::IF_SELF_WINDOW)
	{
		_pWnd = new Wnd;
		h = _pWnd->handle();
	}

	if ((flags & INIT_FLAGS::IF_GRAPHIC_LIBRARY_FLAG) == INIT_FLAGS::IF_DIRECTX11)
		_pCoreRender = new DX11CoreRender;
	else
		_pCoreRender = new GLCoreRender;

	_pCoreRender->Init(handle);

	if ((flags & INIT_FLAGS::IF_WINDOW_FLAG) == INIT_FLAGS::IF_SELF_WINDOW)
	{
		_pWnd->CreateAndShow();
		_pWnd->StartMainLoop();
		_pWnd->Destroy();
	}
	Log("Engine initialized");

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
	std::cout << pStr << std::endl;

	_evLog.Fire(pStr, type);

	return S_OK;
}

API Core::CloseEngine()
{
	_pCoreRender->Free();

	Log("Core::CloseEngine()");

	delete _pResMan;

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
