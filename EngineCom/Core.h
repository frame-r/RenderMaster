#pragma once
#include "Common.h"
#include "EvLog.h"

DEFINE_GUID(CLSID_Core,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);

class Core : public ICore
{
	long _lRef;
	IResourceManager *_pResMan;
	ICoreRender *_pCoreRender;
	EvLog _evLog;

public:

	Core();
	~Core();

	API Init(INIT_FLAGS flags, WinHandle& handle) override;
	API GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type) override;
	API Log(const char *pStr, LOG_TYPE type = LOG_TYPE::LT_NORMAL) override;
	API CloseEngine() override;

	API GetLogPrintedEv(ILogEvent *&pEvent) override;

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;
};
