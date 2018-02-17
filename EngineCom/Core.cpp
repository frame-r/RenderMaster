#include "Core.h"
#include <iostream>

API Core::StartEngine()
{
	std::cout << "Engine started\n";
	return S_OK;
}

API Core::CloseEngine()
{
	std::cout << "Engine closed\n";
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
	InterlockedIncrement(&m_lRef);
	return 0;
}

ULONG Core::Release()
{
	InterlockedDecrement(&m_lRef);
	if (m_lRef == 0)
	{
		delete this;
		return 0;
	}
	else
		return m_lRef;
}
