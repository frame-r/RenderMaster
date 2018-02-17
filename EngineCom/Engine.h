#pragma once
#define INITGUID
#include <Unknwn.h>
#include <iostream>

#define API HRESULT


// {A97B8EB3-93CE-4A45-800D-367084CFB4B1}
DEFINE_GUID(IID_Core,
	0xa97b8eb3, 0x93ce, 0x4a45, 0x80, 0xd, 0x36, 0x70, 0x84, 0xcf, 0xb4, 0xb1);

class ICore : public IUnknown
{
public:

	virtual API StartEngine() = 0;
	virtual API CloseEngine() = 0;
};



//////////////////////
// COM stuff
//////////////////////

inline bool GetEngine(ICore*& pCore)
{
	//cout << "Initializing COM" << endl;

	if (FAILED(CoInitialize(NULL)))
	{
		std::cout << "Unable to initialize COM" << std::endl;
		return false;
	}

	char* szProgID = "RenderMaster.Component.1";
	WCHAR  szWideProgID[128];
	CLSID  clsid;
	long lLen = MultiByteToWideChar(CP_ACP,
		0,
		szProgID,
		(int)strlen(szProgID),
		szWideProgID,
		sizeof(szWideProgID));

	szWideProgID[lLen] = '\0';
	HRESULT hr;
	hr = ::CLSIDFromProgID(szWideProgID, &clsid);
	if (FAILED(hr))
	{
		std::cout.setf(std::ios::hex, std::ios::basefield);
		std::cout << "Unable to get CLSID from ProgID. HR = " << hr << std::endl;
		return false;
	}

	IClassFactory* pCFactory;
	// Get the class factory for the Math class

	hr = CoGetClassObject(clsid,
		CLSCTX_INPROC,
		NULL,
		IID_IClassFactory,
		(void**)&pCFactory);
	if (FAILED(hr))
	{
		std::cout.setf(std::ios::hex, std::ios::basefield);
		std::cout << "Failed to GetClassObject server instance. HR = " << hr << std::endl;
		return false;
	}

	// using the class factory interface create an instance of the
	// component and return the IExpression interface.
	IUnknown* pUnk;
	hr = pCFactory->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);

	//// Release the class factory
	pCFactory->Release();

	if (FAILED(hr))
	{
		std::cout.setf(std::ios::hex, std::ios::basefield);
		std::cout << "Failed to create server instance. HR = " << hr << std::endl;
		return false;
	}

	//cout << "Instance created" << endl;

	pCore = NULL;
	hr = pUnk->QueryInterface(IID_Core, (LPVOID*)&pCore);
	pUnk->Release();

	//hr = CoCreateInstance(CLSID_Math,         // CLSID of coclass
	//	NULL,                    // not used - aggregation
	//	CLSCTX_ALL,    // type of server
	//	IID_IMath,          // IID of interface
	//	(void**)&pCore);

	if (FAILED(hr))
	{
		std::cout << "QueryInterface() for IMath failed" << std::endl;
		return false;
	}
	return true;
}

inline void FreeEngine(ICore *pCore)
{
	pCore->Release();
	//cout << "Shuting down COM" << endl;
	CoUninitialize();
}

