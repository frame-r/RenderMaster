#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <initguid.h>

#include "../EngineCom/Engine.h"

using namespace std;

bool GetEngine(ICore*& pCore);
void FreeEngine(ICore *pCore);


int main(int argc, char *argv[])
{
	ICore* pCore;

	if (GetEngine(pCore))
	{
		pCore->StartEngine();
		pCore->CloseEngine();

		FreeEngine(pCore);
	}
	
	std::getchar();

	return 0;
}


DEFINE_GUID(CLSID_Core1,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);
bool GetEngine(ICore*& pCore)
{
	//cout << "Initializing COM" << endl;

	if (FAILED(CoInitialize(NULL)))
	{
		cout << "Unable to initialize COM" << endl;
		return false;
	}

	char* szProgID = "RenderMaster.Component.1";
	WCHAR  szWideProgID[128];
	CLSID  clsid;
	long lLen = MultiByteToWideChar(CP_ACP,
		0,
		szProgID,
		strlen(szProgID),
		szWideProgID,
		sizeof(szWideProgID));

	szWideProgID[lLen] = '\0';
	HRESULT hr;
	hr = ::CLSIDFromProgID(szWideProgID, &clsid);
	if (FAILED(hr))
	{
		cout.setf(ios::hex, ios::basefield);
		cout << "Unable to get CLSID from ProgID. HR = " << hr << endl;
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
		cout.setf(ios::hex, ios::basefield);
		cout << "Failed to GetClassObject server instance. HR = " << hr << endl;
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
		cout.setf(ios::hex, ios::basefield);
		cout << "Failed to create server instance. HR = " << hr << endl;
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
		cout << "QueryInterface() for IMath failed" << endl;
		return false;
	}
	return true;
}

void FreeEngine(ICore *pCore)
{
	pCore->Release();
	//cout << "Shuting down COM" << endl;
	CoUninitialize();
}
