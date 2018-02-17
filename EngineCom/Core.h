#pragma once
#include "Engine.h"

DEFINE_GUID(CLSID_Core1,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);

class Core : public ICore
{
	long m_lRef;

public:

	virtual API StartEngine() override;
	virtual API CloseEngine() override;

	virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
	virtual STDMETHODIMP_(ULONG) AddRef() override;
	virtual STDMETHODIMP_(ULONG) Release() override;
};