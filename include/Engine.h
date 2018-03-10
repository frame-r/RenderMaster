#pragma once
#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <Unknwn.h>
#include <iostream>

#define API HRESULT

typedef unsigned int uint;
typedef unsigned char uint8;
typedef HWND WinHandle;

#define USE_FBX

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
	class ISubSystem;
	class ILogEvent;
	enum class SUBSYSTEM_TYPE;


	//////////////////////
	// Core
	//////////////////////

	enum class INIT_FLAGS
	{
		IF_WINDOW_FLAG = 0x0000000F,
		IF_SELF_WINDOW = 0x00000001, // engine should create it's own window 
		IF_EXTERN_WINDOW = 0x00000002, // engine uses client's created window		

		IF_GRAPHIC_LIBRARY_FLAG = 0x000000F0,
		IF_OPENGL45 = 0x00000010,
		IF_DIRECTX11 = 0x00000020,		
		
		IF_CONSOLE_FLAG = 0x00000F00,
		IF_NO_CONSOLE = 0x00000100,  // no need create console
		IF_CONSOLE = 0x00000200 // engine should create console		
	};
	DEFINE_ENUM_OPERATORS(INIT_FLAGS)

		enum class LOG_TYPE
	{
		LT_NORMAL,
		LT_WARNING,
		LT_FATAL
	};

	// {A97B8EB3-93CE-4A45-800D-367084CFB4B1}
	DEFINE_GUID(IID_Core,
		0xa97b8eb3, 0x93ce, 0x4a45, 0x80, 0xd, 0x36, 0x70, 0x84, 0xcf, 0xb4, 0xb1);

	class ICore : public IUnknown
	{
	public:

		virtual API Init(INIT_FLAGS flags, WinHandle* handle, const char *pDataPath) = 0;
		virtual API GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type) = 0;
		virtual API GetDataPath(const char *&pStr) = 0;
		virtual API Log(const char *pStr, LOG_TYPE type) = 0;
		virtual API CloseEngine() = 0;

		// Events
		virtual API GetLogPrintedEv(ILogEvent *&pEvent) = 0;
	};


	//////////////////////
	// Common
	//////////////////////

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

	enum class RES_TYPE
	{
		RT_CORE_MESH,
		RT_CORE_TEXTURE,
		RT_CORE_SHADER,

		RT_GAMEOBJECT,
		RT_MODEL
	};

	class IResource
	{
	public:
		virtual API Free() = 0;
		virtual API GetType(RES_TYPE& type) = 0;
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

	enum class DRAW_MODE
	{
		DM_POINTS,
		DM_LINES,
		DM_TRIANGLES,
	};

	struct MeshDataDesc
	{
		MeshDataDesc() : pData(nullptr), number(0), positionOffset(0), positionStride(12),
			texCoordPresented(false), texCoordOffset(0), texCoordStride(0),
			normalsPresented(false), normalOffset(0), normalStride(0) {}

		uint8 *pData;

		// number of vertex
		uint number;

		// At minimum position attribute must be present
		// Stride is step in bytes to move along the array from vertex to vertex 
		// Offset also specified in bytes
		// Stride and offset defines two case:

		// 1) Interleaved
		//
		// x1, y1, z1, UVx1, UVy1, Nx1, Ny1, Nz1,   x2, y2, z2, UVx2, UVy2, Nx2, Ny2, Nz2, ...
		// positionOffset = 0, positionStride = 32,
		// texCoordOffset = 12, texCoordStride = 32,
		// normalOffset = 20, normalStride = 32

		// 2) Tightly packed attributes
		//
		// x1, y2, z1, x2, y2, z2, ...   UVx1, UVy1, UVx2, UVy2, ...  Nx1, Ny1, Nz1, Nx2, Ny2, Nz2, ...
		// positionOffset = 0, positionStride = 12,
		// texCoordOffset = vertexNumber * 12, texCoordStride = 8,
		// normalOffset = vertexNumber * (12 + 8), normalStride = 12

		uint positionOffset;
		uint positionStride;

		bool texCoordPresented;
		uint texCoordOffset;
		uint texCoordStride;

		bool normalsPresented;
		uint normalOffset;
		uint normalStride;
	};

	enum class MESH_INDEX_FORMAT
	{
		MID_NOTHING,
		MID_INT32,
		MID_INT16
	};

	struct MeshIndexDesc
	{
		MeshIndexDesc() : pData(nullptr), number(0), format(MESH_INDEX_FORMAT::MID_NOTHING) {}

		uint8 *pData;
		uint number;
		MESH_INDEX_FORMAT format;
	};

	class ICoreMesh : public IResource
	{
	public:
		virtual API GetNumberOfVertex(uint &vertex) = 0;
	};

	struct ShaderDesc
	{
		const char** pVertStr;
		int vertNumLines;

		const char** pGeomStr;
		int geomNumLines;

		const char** pFragStr;
		int fragNumLines;
	};

	class ICoreShader : public IResource
	{
	};

	class ICoreTexture : public IResource
	{
	};

	class ICoreRender : public ISubSystem
	{
	public:
		virtual API Init(WinHandle* handle) = 0;
		virtual API CreateMesh(ICoreMesh *& pMesh, MeshDataDesc &dataDesc, MeshIndexDesc &indexDesc, DRAW_MODE mode) = 0;
		virtual API CreateShader(ICoreShader *&pShader, ShaderDesc& shaderDesc) = 0;
		virtual API Clear() = 0;
		virtual API Free() = 0;
	};


	//////////////////////
	// Game Objects
	//////////////////////
	class IGameObject : public IResource
	{
		
	};

	class IModel : public IGameObject
	{
	public:
		virtual API GetMesh(ICoreMesh *&pMesh, uint idx) = 0;
		virtual API GetMeshesNumber(uint& number) = 0;
	};


	//////////////////////
	// Resource Manager
	//////////////////////

	enum class DEFAULT_RESOURCE_TYPE
	{
		DRT_PLANE
	};

	class IProgressSubscriber
	{
	public:
		virtual API ProgressChanged(uint i) = 0;
	};

	class IResourceManager : public ISubSystem
	{
	public:

		virtual API LoadModel(IModel *&pMesh, const char *pFileName, IProgressSubscriber *pProgress) = 0;
		virtual API LoadShader(ICoreShader *&pShader, const char* pVertName, const char* pGeomName, const char* pFragName) = 0;
		virtual API CreateDefaultModel(IModel *&pModel, DEFAULT_RESOURCE_TYPE type) = 0;
		virtual API AddToList(IResource *pResource) = 0;
		virtual API GetRefNumber(IResource *pResource, uint& number) = 0;
		virtual API DecrementRef(IResource *pResource) = 0;
		virtual API RemoveFromList(IResource *pResource) = 0;
		virtual API FreeAllResources() = 0;
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
