#pragma once

#include "vector_math.h"
#include "quat.h"

#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <Unknwn.h>

#include <iostream>

// comment this to build without FBX SDK
#define USE_FBX

#define APIRESULT HRESULT
#define API HRESULT __stdcall 

typedef unsigned int uint;
typedef unsigned char uint8;
typedef int uint32;
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

#define DEFINE_EVENT(NAME) \
class NAME ## Subscriber \
{ \
public: \
	virtual API Call() = 0; \
}; \
\
class NAME \
{ \
public: \
	\
	virtual API Subscribe(NAME ## Subscriber *pSubscriber) = 0; \
	virtual API Unsubscribe(NAME ## Subscriber *pSubscriber) = 0; \
};

#define DEFINE_EVENT1(NAME, ARG) \
class NAME ## Subscriber \
{ \
public: \
	virtual API Call(ARG) = 0; \
}; \
\
class NAME \
{ \
public: \
	\
	virtual API Subscribe(NAME ## Subscriber *pSubscriber) = 0; \
	virtual API Unsubscribe(NAME ## Subscriber *pSubscriber) = 0; \
};

#define DEFINE_EVENT2(NAME, ARG1, ARG2) \
class NAME ## Subscriber \
{ \
public: \
	virtual API Call(ARG1, ARG2) = 0; \
}; \
 \
class NAME \
{ \
public: \
 \
	virtual API Subscribe(NAME ## Subscriber *pSubscriber) = 0; \
	virtual API Unsubscribe(NAME ## Subscriber *pSubscriber) = 0; \
};


namespace RENDER_MASTER 
{
	class ISubSystem;
	class ILogEvent;
	class IInitCallback;
	class IUpdateCallback;
	class ICamera;
	class IGameObject;
	enum class SUBSYSTEM_TYPE;


	//////////////////////
	// Core
	//////////////////////

	enum class INIT_FLAGS
	{
		WINDOW_FLAG				= 0x0000000F, 
		EXTERN_WINDOW			= 0x00000002, // engine uses client's created window	

		GRAPHIC_LIBRARY_FLAG	= 0x000000F0,
		OPENGL45				= 0x00000010,
		DIRECTX11				= 0x00000020,		

		CREATE_CONSOLE_FLAG		= 0x00000F00,
		NO_CREATE_CONSOLE		= 0x00000100,  // no need create console
		CREATE_CONSOLE			= 0x00000200 // engine should create console		
	};
	DEFINE_ENUM_OPERATORS(INIT_FLAGS)

	enum class LOG_TYPE
	{
		NORMAL,
		WARNING,
		FATAL
	};

	// {A97B8EB3-93CE-4A45-800D-367084CFB4B1}
	DEFINE_GUID(IID_Core,
		0xa97b8eb3, 0x93ce, 0x4a45, 0x80, 0xd, 0x36, 0x70, 0x84, 0xcf, 0xb4, 0xb1);

	class ICore : public IUnknown
	{
	public:

		virtual API Init(INIT_FLAGS flags, const char *pDataPath, const WinHandle* externHandle) = 0;
		virtual API Start() = 0;
		virtual API RenderFrame(const WinHandle *externHandle, const ICamera *pCamera) = 0;
		virtual API GetSubSystem(OUT ISubSystem **pSubSystem, SUBSYSTEM_TYPE type) = 0;
		virtual API GetDataDir(OUT char **pStr) = 0;
		virtual API GetWorkingDir(OUT char **pStr) = 0;
		virtual API GetInstalledDir(OUT char **pStr) = 0;
		virtual API Log(const char *pStr, LOG_TYPE type) = 0;
		virtual API AddInitCallback(IInitCallback *pCallback) = 0;
		virtual API AddUpdateCallback(IUpdateCallback *pCallback) = 0;
		virtual API CloseEngine() = 0;

		// Events
		virtual API GetLogPrintedEv(OUT ILogEvent **pEvent) = 0;
	};


	//////////////////////
	// Common
	//////////////////////

	enum class SUBSYSTEM_TYPE
	{
		FILESYSTEM,
		CORE_RENDER,
		RENDER,
		RESOURCE_MANAGER,
		INPUT,
		SCENE_MANAGER
	};

	class ISubSystem
	{
	public:
		virtual API GetName(OUT const char **pName) = 0;
	};

	enum class RES_TYPE
	{
		CORE_MESH,
		CORE_TEXTURE,
		CORE_SHADER,
		UNIFORM_BUFFER,
		GAMEOBJECT,
		MODEL,
		CAMERA
	};

	class IResource
	{
	public:
		virtual API Free() = 0;
		virtual API GetType(OUT RES_TYPE *type) = 0;
	};

	class IInitCallback
	{
	public:
		virtual API Init() = 0;
	};

	class IUpdateCallback
	{
	public:
		virtual API Update() = 0;
	};
	

	//////////////////////
	// Events
	//////////////////////

	DEFINE_EVENT(IEvent)
	DEFINE_EVENT1(IPositionEvent, OUT vec3 *pos)
	DEFINE_EVENT1(IRotationEvent, OUT quat *rot)
	DEFINE_EVENT1(IGameObjectEvent, OUT IGameObject *pGameObject)
	DEFINE_EVENT1(IStringEvent, const char *pString)
	DEFINE_EVENT2(ILogEvent, const char *pMessage, LOG_TYPE type)


	//////////////////////
	// Core Render
	//////////////////////

	enum class INPUT_ATTRUBUTE
	{
		CUSTOM		= 0,
		POSITION	= 1 << 0,
		NORMAL		= 1 << 1,
		TEX_COORD	= 1 << 2,
		COLOR		= 1 << 3
	};
	DEFINE_ENUM_OPERATORS(INPUT_ATTRUBUTE)

	enum class VERTEX_TOPOLOGY
	{
		LINES,
		TRIANGLES,
	};

	//
	// Vertex buffer description
	//
	// Position attribute must be present
	// Stride is step in bytes to move along the array from vertex to vertex 
	// Offset also specified in bytes
	// Stride and offset defines two general case:
	//
	// 1) Interleaved
	//
	// x1, y1, z1, UVx1, UVy1, Nx1, Ny1, Nz1,  x2, y2, z2, UVx2, UVy2, Nx2, Ny2, Nz2, ...
	// positionOffset = 0, positionStride = 32,
	// texCoordOffset = 12, texCoordStride = 32,
	// normalOffset = 20, normalStride = 32
	//
	// 2) Tightly packed
	//
	// x1, y2, z1, x2, y2, z2, ...   UVx1, UVy1, UVx2, UVy2, ...  Nx1, Ny1, Nz1, Nx2, Ny2, Nz2, ...
	// positionOffset = 0, positionStride = 12,
	// texCoordOffset = vertexNumber * 12, texCoordStride = 8,
	// normalOffset = vertexNumber * (12 + 8), normalStride = 12

	struct MeshDataDesc
	{
		uint8 *pData{nullptr};

		uint numberOfVertex{0};

		uint positionOffset{0};
		uint positionStride{0};

		bool normalsPresented{false};
		uint normalOffset{0};
		uint normalStride{0};

		bool texCoordPresented{false};
		uint texCoordOffset{0};
		uint texCoordStride{0};

		bool colorPresented{false};
		uint colorOffset{0};
		uint colorStride{0};
	};

	enum class MESH_INDEX_FORMAT
	{
		NOTHING,
		INT32,
		INT16
	};

	enum class SHADER_VARIABLE_TYPE
	{
		INT,
		FLOAT,
		VECTOR3,
		VECTOR4,
		MATRIX3X3,
		MATRIX4X4,
	};

	struct MeshIndexDesc
	{
		uint8 *pData{nullptr};
		uint number{0}; // number of index
		MESH_INDEX_FORMAT format{MESH_INDEX_FORMAT::NOTHING};
	};

	class ICoreMesh : public IResource
	{
	public:
		virtual API GetNumberOfVertex(OUT uint *number) = 0;
		virtual API GetAttributes(OUT INPUT_ATTRUBUTE *attribs) = 0;
		virtual API GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) = 0;
	};

	struct ShaderText
	{
		const char* pVertText{nullptr};
		const char* pGeomText{nullptr};
		const char* pFragText{nullptr};
	};

	class ICoreShader : public IResource
	{
	};

	class ICoreTexture : public IResource
	{
	};

	class IUniformBuffer : public IResource
	{
	};

	class ICoreRender : public ISubSystem
	{
	public:
		
		virtual API Init(const WinHandle* handle) = 0;
		virtual API Free() = 0;
		virtual API MakeCurrent(const WinHandle* handle) = 0;
		virtual API SwapBuffers() = 0;

		virtual API PushStates() = 0;
		virtual API PopStates() = 0;

		virtual API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) = 0;
		virtual API CreateShader(OUT ICoreShader **pShader, const ShaderText* shaderDesc) = 0;
		virtual API SetShader(const ICoreShader *pShader) = 0;
		virtual API CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size) = 0;
		virtual API SetUniform(IUniformBuffer *pBuffer, const void *pData) = 0;
		virtual API SetUniformBufferToShader(IUniformBuffer *pBuffer, uint slot) = 0;
		virtual API SetMesh(const ICoreMesh* mesh) = 0;
		virtual API Draw(ICoreMesh *mesh) = 0;
		virtual API SetDepthState(int enabled) = 0;
		virtual API SetViewport(uint w, uint h) = 0;
		virtual API GetViewport(OUT uint* w, OUT uint* h) = 0;
		virtual API Clear() = 0;
	};

	//////////////////////
	// Render
	//////////////////////

	struct ShaderRequirement
	{
		INPUT_ATTRUBUTE attributes{INPUT_ATTRUBUTE::CUSTOM};
		bool alphaTest{false};

		size_t operator()(const ShaderRequirement& k) const
		{
			return ((int)k.alphaTest + 1) * (int)k.attributes;
		}
		bool operator==(const ShaderRequirement &other) const
		{
			return attributes == other.attributes && alphaTest == other.alphaTest;
		}
	};


	class IRender : public ISubSystem
	{
	public:
		virtual API GetShader(OUT ICoreShader **pShader, const ShaderRequirement *shaderReq) = 0;
	};



	//////////////////////
	// Game Objects
	//////////////////////
	
	class IGameObject : public IResource
	{
	public:
		virtual API GetID(OUT int *id) = 0;
		virtual API GetName(OUT const char **pName) = 0;
		virtual API SetName(const char *pName) = 0;
		virtual API SetPosition(const vec3 *pos) = 0;
		virtual API SetRotation(const quat *rot) = 0;
		virtual API SetScale(const vec3 *scale) = 0;
		virtual API GetPosition(OUT vec3 *pos) = 0;
		virtual API GetRotation(OUT quat *rot) = 0;
		virtual API GetScale(OUT vec3 *scale) = 0;
		virtual API GetModelMatrix(OUT mat4 *mat) = 0;
		virtual API GetInvModelMatrix(OUT mat4 *mat) = 0;

		// Events
		virtual API GetNameEv(OUT IStringEvent **pEvent) = 0;
		virtual API GetPositionEv(OUT IPositionEvent **pEvent) = 0;
		virtual API GetRotationEv(OUT IRotationEvent **pEvent) = 0;
	};

	class ICamera : public IGameObject
	{
	public:
		virtual API GetViewProjectionMatrix(OUT mat4 *mat, float aspect) = 0;
	};

	class IModel : public IGameObject
	{
	public:
		virtual API GetMesh(OUT ICoreMesh **pMesh, uint idx) = 0;
		virtual API GetNumberOfMesh(OUT uint *number) = 0;
	};

	//////////////////////
	// Scene Manager
	//////////////////////

	class ISceneManager : public ISubSystem
	{
	public:
		virtual API SaveScene(const char *name) = 0;
		virtual API GetDefaultCamera(OUT ICamera **pCamera) = 0;
		virtual API AddRootGameObject(IGameObject* pGameObject) = 0;
		virtual API GetChilds(OUT uint *number, IGameObject *parent) = 0;
		virtual API GetChild(OUT IGameObject **pGameObject, IGameObject *parent, uint idx) = 0;

		//events
		virtual API GetGameObjectAddedEvent(OUT IGameObjectEvent **pEvent) = 0;
	};


	//////////////////////
	// Resource Manager
	//////////////////////

	enum class DEFAULT_RES_TYPE
	{
		CUSTOM,
		PLANE,
		AXES,
		AXES_ARROWS,
		GRID
	};

	class IProgressSubscriber
	{
	public:
		virtual API ProgressChanged(uint i) = 0;
	};

	class IResourceManager : public ISubSystem
	{
	public:

		virtual API LoadModel(OUT IModel **pMesh, const char *pFileName, IProgressSubscriber *pProgress) = 0;
		virtual API LoadShaderText(OUT ShaderText *pShader, const char *pVertName, const char *pGeomName, const char *pFragName) = 0;
		virtual API GetDefaultResource(OUT IResource **pRes, DEFAULT_RES_TYPE type) = 0;
		virtual API GetNumberOfResources(OUT uint *number) = 0;
		virtual API AddToList(IResource *pResource) = 0;
		virtual API GetRefNumber(OUT uint *number, const IResource *pResource) = 0;
		virtual API DecrementRef(IResource *pResource) = 0;
		virtual API RemoveFromList(IResource *pResource) = 0;
		virtual API FreeAllResources() = 0;
	};

	
	//////////////////////
	// Filesystem
	//////////////////////

	enum class FILE_OPEN_MODE
	{
		READ	= 1 << 0,
		WRITE	= 1 << 1,
		APPEND	= 1 << 2,
		BINARY	= 1 << 3,
	};
	DEFINE_ENUM_OPERATORS(FILE_OPEN_MODE)

	class IFile
	{
	public:

		virtual API Read(OUT uint8 *pMem, uint bytes) = 0;
		virtual API ReadStr(OUT char *pStr, OUT uint *str_bytes) = 0;
		virtual API IsEndOfFile(OUT int *eof) = 0;
		virtual API Write(const uint8 *pMem, uint bytes) = 0;
		virtual API WriteStr(const char *pStr) = 0;
		virtual API FileSize(OUT uint *size) = 0;
		virtual API CloseAndFree() = 0;
	};

	class IFileSystem : public ISubSystem
	{
	public:

		virtual API OpenFile(OUT IFile **pFile, const char *fullPath, FILE_OPEN_MODE mode) = 0;
		virtual API FileExist(const char *fullPath, OUT int *exist) = 0;
		virtual API DirectoryExist(const char *fullPath, OUT int *exist) = 0;
	};


	//////////////////////
	// Input
	//////////////////////

	enum class KEYBOARD_KEY_CODES
	{
		KEY_UNKNOWN			= 0x0,

		KEY_ESCAPE			= 0x01,
		KEY_TAB				= 0x0F,
		KEY_GRAVE			= 0x29,
		KEY_CAPSLOCK		= 0x3A,
		KEY_BACKSPACE		= 0x0E,
		KEY_RETURN			= 0x1C,
		KEY_SPACE			= 0x39,
		KEY_SLASH			= 0x35,
		KEY_BACKSLASH		= 0x2B,

		KEY_SYSRQ			= 0xB7,
		KEY_SCROLL			= 0x46,
		KEY_PAUSE			= 0xC5,

		KEY_INSERT			= 0xD2,
		KEY_DELETE			= 0xD3,
		KEY_HOME			= 0xC7,
		KEY_END				= 0xCF,
		KEY_PGUP			= 0xC9,
		KEY_PGDN			= 0xD1,

		KEY_LSHIFT			= 0x2A,
		KEY_RSHIFT			= 0x36,
		KEY_LALT			= 0x38,
		KEY_RALT			= 0xB8,
		KEY_LWIN_OR_CMD		= 0xDB,
		KEY_RWIN_OR_CMD		= 0xDC,
		KEY_LCONTROL		= 0x1D,
		KEY_RCONTROL		= 0x9D,

		KEY_UP				= 0xC8,
		KEY_RIGHT			= 0xCD,
		KEY_LEFT			= 0xCB,
		KEY_DOWN			= 0xD0,

		KEY_1				= 0x02,
		KEY_2				= 0x03,
		KEY_3				= 0x04,
		KEY_4				= 0x05,
		KEY_5				= 0x06,
		KEY_6				= 0x07,
		KEY_7				= 0x08,
		KEY_8				= 0x09,
		KEY_9				= 0x0A,
		KEY_0				= 0x0B,

		KEY_F1				= 0x3B,
		KEY_F2				= 0x3C,
		KEY_F3				= 0x3D,
		KEY_F4				= 0x3E,
		KEY_F5				= 0x3F,
		KEY_F6				= 0x40,
		KEY_F7				= 0x41,
		KEY_F8				= 0x42,
		KEY_F9				= 0x43,
		KEY_F10				= 0x44,
		KEY_F11				= 0x57,
		KEY_F12				= 0x58,

		KEY_Q				= 0x10,
		KEY_W				= 0x11,
		KEY_E				= 0x12,
		KEY_R				= 0x13,
		KEY_T				= 0x14,
		KEY_Y				= 0x15,
		KEY_U				= 0x16,
		KEY_I				= 0x17,
		KEY_O				= 0x18,
		KEY_P				= 0x19,
		KEY_A				= 0x1E,
		KEY_S				= 0x1F,
		KEY_D				= 0x20,
		KEY_F				= 0x21,
		KEY_G				= 0x22,
		KEY_H				= 0x23,
		KEY_J				= 0x24,
		KEY_K				= 0x25,
		KEY_L				= 0x26,
		KEY_Z				= 0x2C,
		KEY_X				= 0x2D,
		KEY_C				= 0x2E,
		KEY_V				= 0x2F,
		KEY_B				= 0x30,
		KEY_N				= 0x31,
		KEY_M				= 0x32,

		KEY_MINUS			= 0x0C,
		KEY_PLUS			= 0x0D,
		KEY_LBRACKET		= 0x1A,
		KEY_RBRACKET		= 0x1B,

		KEY_SEMICOLON		= 0x27,
		KEY_APOSTROPHE		= 0x28,

		KEY_COMMA			= 0x33,
		KEY_PERIOD			= 0x34,

		KEY_NUMPAD0			= 0x52,
		KEY_NUMPAD1			= 0x4F,
		KEY_NUMPAD2			= 0x50,
		KEY_NUMPAD3			= 0x51,
		KEY_NUMPAD4			= 0x4B,
		KEY_NUMPAD5			= 0x4C,
		KEY_NUMPAD6			= 0x4D,
		KEY_NUMPAD7			= 0x47,
		KEY_NUMPAD8			= 0x48,
		KEY_NUMPAD9			= 0x49,
		KEY_NUMPADPERIOD	= 0x53,
		KEY_NUMPADENTER		= 0x9C,
		KEY_NUMPADSTAR		= 0x37,
		KEY_NUMPADPLUS		= 0x4E,
		KEY_NUMPADMINUS		= 0x4A,
		KEY_NUMPADSLASH		= 0xB5,
		KEY_NUMLOCK			= 0x45,
	};

	enum class MOUSE_BUTTON
	{
		LEFT,
		RIGHT,
		MIDDLE
	};

	class IInput : public ISubSystem
	{
	public:

		virtual API IsKeyPressed(OUT int *isPressed, KEYBOARD_KEY_CODES key) = 0;
		virtual API IsMoisePressed(OUT int *isPressed, MOUSE_BUTTON type) = 0;
		virtual API GetMouseDeltaPos(OUT vec2 *dPos) = 0;

	};

	
	//////////////////////
	// COM stuff
	//////////////////////

	namespace
	{
		const TCHAR *pErrorMessage = TEXT("success");
	}

	inline bool GetCore(OUT ICore** pCore)
	{
		if (FAILED(CoInitialize(NULL)))
		{
			pErrorMessage = TEXT("Unable to initialize COM");
			std::cout << pErrorMessage << std::endl;
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
			//TCHAR buf[260];
			//swprintf(buf, TEXT("Unable to get CLSID from ProgID. HR = %02X"), hr);
			pErrorMessage = TEXT("Unable to get CLSID from ProgID RenderMaster.Component.1");
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << pErrorMessage << "HR = " << hr << std::endl;
			return false;
		}

		IClassFactory* pCFactory;
		// Get the class factory for the Core class

		hr = CoGetClassObject(clsid,
			CLSCTX_INPROC,
			NULL,
			IID_IClassFactory,
			(void**)&pCFactory);
		if (FAILED(hr))
		{
			pErrorMessage = TEXT("Failed to GetClassObject server instance. HR = ");
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << pErrorMessage << hr << std::endl;
			return false;
		}

		IUnknown* pUnk;
		hr = pCFactory->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);

		// Release the class factory
		pCFactory->Release();

		if (FAILED(hr))
		{
			pErrorMessage = TEXT("Failed to create server instance. HR = ");
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << pErrorMessage << hr << std::endl;
			return false;
		}
		
		*pCore = NULL;
		hr = pUnk->QueryInterface(IID_Core, (LPVOID*)pCore);
		pUnk->Release();
		
		if (FAILED(hr))
		{
			pErrorMessage = TEXT("QueryInterface() for IID_Core failed");
			std::cout << pErrorMessage << std::endl;
			return false;
		}
		return true;
	}

	inline void FreeCore(ICore *pCore)
	{		
		pCore->Release();
		
		//Shuting down COM;
		CoUninitialize();
	}

}
