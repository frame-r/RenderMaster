#pragma once
#define INITGUID
#include <Unknwn.h>
#include <iostream>

#define API HRESULT

typedef unsigned int uint;
typedef HWND WinHandle;

#define DEFINE_ENUM_OPERATORS(ENUM_NAME) \
inline ENUM_NAME operator|(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<int>(a) | static_cast<int>(b)); \
} \
inline ENUM_NAME operator&(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<int>(a) & static_cast<int>(b)); \
}

namespace RENDER_MASTER
{
	enum class INIT_FLAGS
	{
		IF_SELF_WINDOW = 0x00000000, // engine need create it's own window 
		IF_EXTERN_WINDOW = 0x00000001,

		IF_OPENGL45 = 0x00000000,
		IF_DIRECTX11 = 0x00000010
	};
	DEFINE_ENUM_OPERATORS(INIT_FLAGS)

	enum class LOG_TYPE
	{
		LT_NORMAL,
		LT_WARNING,
		LT_FATAL
	};

	enum class SUBSYSTEM_TYPE
	{
		ST_CORE_RENDER,
		ST_RESOURCE_MANAGER
	};

	class ISubSystem
	{
	public:

		virtual API GetName(const char *&pName) = 0;
	};


	//////////////////////
	// Events
	//////////////////////

	class ILogEventSubscriber
	{
	public:
		virtual API Call(const char *pStr, LOG_TYPE type) = 0;
	};

	class ILogEvent
	{
	public:

		virtual API Subscribe(ILogEventSubscriber *pSubscriber) = 0;
		virtual API Unsubscribe(ILogEventSubscriber *pSubscriber) = 0;
	};


	class IEventSubscriber
	{
	public:
		virtual API Call() = 0;
	};

	class IEvent
	{
	public:

		virtual API Subscribe(IEventSubscriber *pSubscriber) = 0;
		virtual API Unsubscribe(IEventSubscriber *pSubscriber) = 0;
	};


	//////////////////////
	// Core Render stuff
	//////////////////////

	class IMesh
	{
	public:
		virtual API GetVertexCount(uint &vertex) = 0;
	};

	class IShader
	{
	public:
		virtual API Init() = 0;
	};

	class ITexture
	{
	public:
		virtual API Init() = 0;
	};

	class ICoreRender : public ISubSystem
	{
	public:
		virtual API Init(WinHandle& handle) = 0;
		virtual API Clear() = 0;
		virtual API Free() = 0;
	};


	//////////////////////
	// Resource Manager
	//////////////////////

	class IProgressSubscriber
	{
	public:

		virtual API ProgressChanged(uint i) = 0; // i = 0...100
	};

	class IResourceManager : public ISubSystem
	{
	public:

		virtual API LoadMesh(IMesh *&pMesh, const char *pFileName, IProgressSubscriber *pPregress) = 0;
	};


	//////////////////////
	// Core
	//////////////////////

	// {A97B8EB3-93CE-4A45-800D-367084CFB4B1}
	DEFINE_GUID(IID_Core,
		0xa97b8eb3, 0x93ce, 0x4a45, 0x80, 0xd, 0x36, 0x70, 0x84, 0xcf, 0xb4, 0xb1);

	class ICore : public IUnknown
	{
	public:

		virtual API Init(INIT_FLAGS flags, WinHandle& handle) = 0;
		virtual API GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type) = 0;
		virtual API Log(const char *pStr, LOG_TYPE type) = 0;
		virtual API CloseEngine() = 0;

		// Events
		virtual API GetLogPrintedEv(ILogEvent *&pEvent) = 0;
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

		const char* szProgID = "RenderMaster.Component.1";
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

}
