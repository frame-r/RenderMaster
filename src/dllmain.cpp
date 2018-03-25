#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Core.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

EXTERN_C  HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
	HRESULT hr;
	CoreClassFactory *pCF;

	pCF = 0;

	// Make sure the CLSID is for our Expression component
	if (rclsid != CLSID_Core)
		return(E_FAIL);

	pCF = new CoreClassFactory;

	if (pCF == 0)
		return(E_OUTOFMEMORY);

	hr = pCF->QueryInterface(riid, ppv);

	// Check for failure of QueryInterface
	if (FAILED(hr))
	{
		delete pCF;
		pCF = 0;
	}

	return hr;
}

EXTERN_C HRESULT STDAPICALLTYPE DllCanUnloadNow(void)
{
	return S_OK;
}

