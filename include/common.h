#pragma once

// Global defines.
//#define USE_FBX // Uncommnent to compile with FBX SDK


#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Avoid adding headers to this list.
// If you want use stack, map or some other from STL
// then use it only in .cpp
#include <utility>
#include <memory>
#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>
#include <functional>
#include <fstream>
#include <assert.h>
#include <mutex>
#include <string>
#include <variant>
#include <algorithm>
//

#include "vector_math.h"

#include <windows.h>

#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#ifdef _DLL_EXPORTS
	#define DLLEXPORT __declspec(dllexport)
 #else
	#define DLLEXPORT __declspec(dllimport)
#endif

#ifdef WIN32
	#define NOVTABLE __declspec(novtable)
#else
	#define NOVTABLE
#endif

class ResourceManager;
class GameObject;
class Core;
class MaterialManager;
class ICoreRender;
class ICoreTexture;
class FileSystem;
class Core;
class Console;
class MainWindow;
class Camera;
class Input;
class Render;
class RenderPathBase;
class RenderPathRealtime;
class RenderPathPathTracing;
class IProfilerCallback;
class File;
struct FileMapping;
class Camera;
class Mesh;
class Model;
class Material;
class StructuredBuffer;
struct ShaderRequirement;
class Shader;
class Texture;
class Light;
class Material;
struct GenericMaterial;


enum class WINDOW_MESSAGE;
enum class LOG_TYPE;

typedef HWND WindowHandle;
typedef int uint32;
typedef unsigned int uint;
typedef unsigned char uint8;
typedef void (*ConsoleCallback)(const char *, LOG_TYPE);
typedef void (*ObjectCallback)(GameObject*);
typedef void (*WindowCallback)(WINDOW_MESSAGE, uint32, uint32, void*);
typedef void (*ProgressCallback)(int);

extern Core *_core;

#define CORE_RENDER _core->GetCoreRender()
#define RENDER _core->GetRender()
#define FS _core->GetFilesystem()
#define RES_MAN _core->GetResourceManager()
#define MAT_MAN _core->GetMaterialManager()
#define INPUT _core->GetInput()

#define SHADER_DIR "standard\\shaders\\"
#define TEXTURES_DIR "standard\\textures\\"
#define GENERIC_MATERIAL_EXT ".genericmat"
#define USER_MATERIAL_EXT ".mat"

inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		abort();
}

template<typename T>
using SharedPtr = std::shared_ptr<T>;

std::string msaa_to_string(int samples);
std::string ConvertFromUtf16ToUtf8(const std::wstring& wstr);
std::wstring ConvertFromUtf8ToUtf16(const std::string& str);

#ifdef _WIN32
	typedef std::wstring mstring;
	inline std::string NativeToUTF8(const std::wstring& wstr)
	{
		return ConvertFromUtf16ToUtf8(wstr);
	}
	inline mstring UTF8ToNative(const std::string& str)
	{
		return ConvertFromUtf8ToUtf16(str);
	}
#else
	typedef std::string mstring;
	#define NativeToUTF8(ARG) ARG
	#define UTF8ToNative(ARG) ARG
#endif

#define DEFINE_ENUM_OPERATORS(ENUM_NAME) \
inline ENUM_NAME operator|(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<int>(a) | static_cast<int>(b)); \
} \
inline ENUM_NAME operator&(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<int>(a) & static_cast<int>(b)); \
}

class IProfilerCallback
{
public:
	virtual uint getNumLines() = 0;
	virtual std::string getString(uint i) = 0;
};

std::string fileExtension(const std::string& path);

namespace Engine
{
	struct CameraData
	{
		mat4 ViewMat;
		mat4 ProjMat;
		float verFullFovInRadians;
	};
}
// Engine enums

enum class INIT_FLAGS
{
	NONE					= 0x00000000,

	WINDOW_FLAG				= 0x0000000F, 
	EXTERN_WINDOW			= 0x00000001, // engine uses client's created window, i.e doesn't create it's own
	
	GRAPHIC_LIBRARY_FLAG	= 0x000000F0,
	OPENGL45				= 0x00000010,
	DIRECTX11				= 0x00000020,
	
	CREATE_CONSOLE_FLAG		= 0x00000F00,
	CREATE_CONSOLE			= 0x00000200, // engine should create console window
	
	MSAA_FLAG				= 0x0000F000,
	MSAA_2X					= 0x00001000,
	MSAA_4X					= 0x00002000,
	MSAA_8X					= 0x00003000,
	MSAA_16X				= 0x00004000,
	MSAA_32X				= 0x00005000,

	VSYNC_FLAG				= 0x000F0000,
	VSYNC_ON				= 0x00010000, // by default
	VSYNC_OFF				= 0x00020000,
};
DEFINE_ENUM_OPERATORS(INIT_FLAGS)

enum class LOG_TYPE
{
	NORMAL,
	WARNING,
	CRITICAL,
	FATAL
};

enum class FILE_OPEN_MODE
{
	READ	= 1 << 0,
	WRITE	= 1 << 1,
	APPEND	= 1 << 2,
	BINARY	= 1 << 3,
};
DEFINE_ENUM_OPERATORS(FILE_OPEN_MODE)

enum class WINDOW_MESSAGE
{
	SIZE,
	KEY_DOWN,
	KEY_UP,
	MOUSE_MOVE,
	MOUSE_DOWN,
	MOUSE_UP,
	WINDOW_DEACTIVATED,
	WINDOW_ACTIVATED,
	APPLICATION_DEACTIVATED,
	APPLICATION_ACTIVATED,
	WINDOW_MINIMIZED,
	WINDOW_UNMINIMIZED,
	WINDOW_REDRAW,
	WINDOW_CLOSE
};

enum class PASS
{
	ALL = -1,
	DEFERRED,
	ID,
	WIREFRAME,
	FORWARD,
	COUNT
};

enum class BLEND_FACTOR
{
	NONE = 0,
	ZERO,
	ONE,
	SRC_COLOR,
	ONE_MINUS_SRC_COLOR,
	SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA,
	DEST_ALPHA,
	ONE_MINUS_DEST_ALPHA,
	DEST_COLOR,
	ONE_MINUS_DEST_COLOR,
};

enum class CULLING_MODE
{
	NONE = 0,
	FRONT,
	BACK	
};

enum class FILLING_MODE
{
	WIREFRAME = 2,
	SOLID = 3,
};
enum class DEPTH_FUNC
{
	NEVER = 1,
	LESS = 2,
	EQUAL = 3,
	LESS_EQUAL = 4,
	GREATER = 5,
	NOT_EQUAL = 6,
	GREATER_EQUAL = 7,
	ALWAYS = 8
};

enum class TEXTURE_FORMAT
{
	// normalized
	R8,
	RG8,
	RGBA8,
	BGRA8,

	// float
	R16F,
	RG16F,
	RGBA16F,
	R32F,
	RG32F,
	RGBA32F,

	// integer
	R32UI,

	// compressed
	DXT1,
	DXT3,
	DXT5,

	// depth/stencil
	D24S8,

	UNKNOWN
};
enum class TEXTURE_CREATE_FLAGS : uint32_t
{
	NONE					= 0x00000000,

	FILTER					= 0x0000000F,
	FILTER_POINT			= 1, // magn = point,	min = point,	mip = point
	FILTER_BILINEAR			= 2, // magn = linear,	min = linear,	mip = point
	FILTER_TRILINEAR		= 3, // magn = linear,	min = linear,	mip = lenear
	FILTER_ANISOTROPY_2X	= 4,
	FILTER_ANISOTROPY_4X	= 5,
	FILTER_ANISOTROPY_8X	= 6,
	FILTER_ANISOTROPY_16X	= 7,


	COORDS					= 0x00000F00,
	COORDS_WRAP				= 1 << 8,
	// TODO
	//COORDS_MIRROR
	//COORDS_CLAMP
	//COORDS_BORDER
	

	USAGE					= 0x0000F000,
	USAGE_RENDER_TARGET		= 1 << 12,
	USAGE_UNORDRED_ACCESS	= 1 << 13,


	MSAA					= 0x000F0000,
	MSAA_2x					= 2 << 16,
	MSAA_4x					= 3 << 16,
	MSAA_8x					= 4 << 16,


	MISC					= 0xF0000000,
	GENERATE_MIPMAPS		= 1 << 28,
};
DEFINE_ENUM_OPERATORS(TEXTURE_CREATE_FLAGS)

enum class TEXTURE_TYPE
{
	TYPE_2D					= 0x00000001,
	//TYPE_3D					= 0x00000001,
	TYPE_CUBE				= 0x00000002,
	//TYPE_2D_ARRAY			= 0x00000003,
	//TYPE_CUBE_ARRAY			= 0x00000004
};

enum class INPUT_ATTRUBUTE
{
	UNKNOWN		= 0,
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

enum class LIGHT_TYPE
{
	DIRECT,
	AREA
};

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

enum class VIEW_MODE
{
	FINAL,
	NORMAL,
	ALBEDO,
	DIFFUSE_LIGHT,
	SPECULAR_LIGHT,
	VELOCITY,
	COLOR_REPROJECTION
};

enum class ENVIRONMENT_TYPE
{
	CUBEMAP,
	ATMOSPHERE,
	NONE
};

enum class PREV_TEXTURES
{
	UNKNOWN,
	COLOR,
	PATH_TRACING_HDR
};

DXGI_FORMAT engToD3DDSVFormat(TEXTURE_FORMAT format);

struct MeshDataDesc
{
	uint numberOfVertex{0};

	uint positionOffset{0};
	uint positionStride{0};

	bool normalsPresented{false};
	uint normalOffset{0};
	uint normalStride{0};

	bool tangentPresented{false};
	uint tangentOffset{0};
	uint tangentStride{0};

	bool binormalPresented{false};
	uint binormalOffset{0};
	uint binormalStride{0};

	bool texCoordPresented{false};
	uint texCoordOffset{0};
	uint texCoordStride{0};

	bool colorPresented{false};
	uint colorOffset{0};
	uint colorStride{0};

	uint8 *pData{nullptr};
};

#pragma pack(push, 1)
struct MeshHeader // 128 bytes
{
	char magic[2];
	char version;
	char attributes; // 0 - positions, 1 - normals, 2 - uv, 3 - tangent, 4 - binormal, 5 -color,
	uint32_t __future1[2];
	uint32_t numberOfVertex;
	uint32_t positionOffset;
	uint32_t positionStride;
	uint32_t normalOffset;
	uint32_t normalStride;
	uint32_t tangentOffset;
	uint32_t tangentStride;
	uint32_t binormalOffset;
	uint32_t binormalStride;
	uint32_t uvOffset;
	uint32_t uvStride;
	uint32_t colorOffset;
	uint32_t colorStride;
	float minX;
	float maxX;
	float minY;
	float maxY;
	float minZ;
	float maxZ;
	uint32_t __future2[10];
};
#pragma pack(pop)

enum class MESH_INDEX_FORMAT
{
	NONE,
	INT32,
	INT16
};

struct MeshIndexDesc
{
	uint8 *pData{nullptr};
	uint number{0}; // number of indexes
	MESH_INDEX_FORMAT format{MESH_INDEX_FORMAT::NONE};
};

struct ShaderInitData
{
	void *pointer;
	unsigned char *bytecode;
	size_t size;
};

#pragma pack(push, 1)
struct RaytracingTriangle
{
	vec4 p0, p1, p2;
	vec4 n;
	uint materialID;
};
#pragma pack(pop)

struct RaytracingData
{	
	std::vector<RaytracingTriangle> triangles;

public:
	RaytracingData(size_t len) : triangles(len) {}
	size_t size() { return triangles.size(); }
};

enum class SHADER_TYPE
{
	SHADER_VERTEX,
	SHADER_GEOMETRY,
	SHADER_FRAGMENT,
	SHADER_COMPUTE
};

enum class ERROR_COMPILE_SHADER
{
	NONE,
	VERTEX,
	FRAGMENT,
	GEOM,
	COMP
};

enum BUFFER_USAGE
{
	CPU_WRITE,
	GPU_READ
};

template<typename... Args>
class Signal
{
	typedef void(*SignalCallback)(Args...);
	std::vector<SignalCallback> functions_;
public:	

	void Add(SignalCallback func)
	{
		functions_.push_back(func);
	}
	void Erase(SignalCallback func)
	{
		functions_.erase(std::remove(functions_.begin(), functions_.end(), func), functions_.end());
	}
	void Invoke(const Args&... args) const
	{
		for (auto f : functions_)
			f(args...);
	}
};


// Random

int currentTime();
bool isIsFree(int id);
void addId(int id);

template<class T>
class RandomInstance
{
	int initialized = 0;
	int seed = 0;

	unsigned int random8()
	{
		if (initialized == 0)
		{
			seed = currentTime();
			initialized = 1;
		}
		seed = seed * 1664525 + 1013904223;
		return (int)((seed >> 20) & 0xff);
	}

public:
	int getRandomInt()
	{
		int newid;
		do
		{
			newid = random8();
			newid |= random8() << 8;
			newid |= random8() << 16;
			newid |= (random8() & 0x7f) << 24;
		} while (!isIsFree(newid) && newid == 0);

		addId(newid);

		return newid;
	}
};


// Resources

template<typename T>
class IResource
{
public:
	virtual ~IResource() = default;
	virtual void addRef() = 0;
	virtual void decRef() = 0;
	virtual int getRefs() const = 0;
	virtual T *get() = 0;
	virtual std::string& getPath() = 0;
	virtual void free() = 0;
	virtual bool isLoaded() = 0;
};

template<typename T>
class Resource : public IResource<T>
{
protected:
	std::string path_;
	std::unique_ptr<T> pointer_;
	int refs_{0};
	bool loadingFailed{false};
	uint64_t frame_{};

	virtual T* create() = 0;

public:
	Resource(const std::string& path);
	~Resource() = default;

	void addRef() override { refs_++; }
	void decRef() override { refs_--; }
	int getRefs() const override { return refs_; }
	std::string& getPath() override { return path_; }
	T* get() override;
	void free() override { pointer_ = nullptr; }
	bool isLoaded() override { return static_cast<bool>(pointer_); }
	uint64_t frame() { return frame_; }
	size_t getVideoMemoryUsage()
	{
		if (pointer_)
			return pointer_->GetVideoMemoryUsage();
		return 0;
	}
};

template<typename T>
class StreamPtr
{
	IResource<T> *resource_{nullptr};

	inline void grab()
	{
		if (resource_)
			resource_->addRef();
	}

public:
	StreamPtr() = default;
	StreamPtr(IResource<T> *resource) : resource_(resource)
	{
		grab();
	}
	StreamPtr(StreamPtr& r)
	{
		resource_ = r.resource_;
		grab();
	}

	void release();
	StreamPtr& operator=(StreamPtr& r)
	{
		release();
		resource_ = r.resource_;
		grab();

		return *this;
	}
	StreamPtr(StreamPtr&& r)
	{
		resource_ = r.resource_;
		r.resource_ = nullptr;
	}
	StreamPtr& operator=(StreamPtr&& r)
	{
		release();
		resource_ = r.resource_;
		r.resource_ = nullptr;

		return *this;
	}
	~StreamPtr()
	{
		release();
	}
	T *get()
	{
		return resource_ ? resource_->get() : nullptr;
	}
	bool isLoaded()
	{
		return resource_ ? resource_->isLoaded() : false;
	}
	const std::string& path()
	{
		static std::string empty;
		if (!resource_)
			return empty;
		return resource_->getPath();
	}
};

// texture
size_t blockSize(TEXTURE_FORMAT compressedFormat);
size_t bytesPerPixel(TEXTURE_FORMAT format);
bool isColorFormat(TEXTURE_FORMAT format);
bool isCompressedFormat(TEXTURE_FORMAT format);
std::unique_ptr<uint8_t[]> convertRGBtoRGBA(const uint8_t* dataIn, uint w, uint h, bool mipmaps, bool bgrTorgb);

// memory
std::string bytesToMBytes(size_t bytes);

// Math
mat3 DLLEXPORT &orthonormalize(mat3 &ret, const mat3 &m);
void DLLEXPORT decompositeTransform(const mat4 &transform, vec3 &t, quat &r, vec3 &s);
void DLLEXPORT compositeTransform(mat4& ret, const vec3& t, const quat& r, const vec3& s);
mat4 DLLEXPORT perspectiveRH_ZO(float fov, float aspect, float zNear, float zFar);
inline vec3 GetRightDirection(const mat4& ModelMat) { return vec3(ModelMat.Column(0)); } // Returns local X vector in world space
inline vec3 GetForwardDirection(const mat4& ModelMat) { return vec3(ModelMat.Column(1)); } // Returns local Y vector in world space
inline vec3 GetBackDirection(const mat4& ModelMat) { return -vec3(ModelMat.Column(2)); } // Returns local -Z vector in world space

// Hash
unsigned short Crc16(unsigned char* pcBlock, unsigned short len);


