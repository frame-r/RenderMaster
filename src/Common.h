#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "Engine.h"

using namespace RENDER_MASTER;
namespace WRL = Microsoft::WRL;

using std::unique_ptr;
using std::vector;
using std::string;

#define SHADER_DIR "src\\shaders"

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

typedef void(*WindowMessageCallback)(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);

template<typename Char>
std::basic_string<Char> ToLowerCase(std::basic_string<Char> str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

#ifdef _DEBUG
#define DEFINE_DEBUG_LOG_HELPERS(CORE_REF) \
	namespace { \
	template <typename... Arguments> \
	void DEBUG_LOG(const char *pStr, LOG_TYPE type, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, type, args...); \
	} \
	template <typename... Arguments> \
	void DEBUG_LOG_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::NORMAL, args...); \
	} \
	}
#else
#define DEFINE_DEBUG_LOG_HELPERS(CORE_REF) \
	namespace { \
	template <typename... Arguments> \
	void DEBUG_LOG(const char *pStr, LOG_TYPE type, Arguments ... args) \
	{ \
	} \
	template <typename... Arguments> \
	void DEBUG_LOG_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
	} \
	}
#endif

#define DEFINE_LOG_HELPERS(CORE_REF) \
	namespace { \
	void LOG(const char *pStr, LOG_TYPE type = LOG_TYPE::NORMAL) \
	{ \
		CORE_REF->Log(pStr, type); \
	} \
	void LOG_WARNING(const char *pStr) \
	{ \
		CORE_REF->Log(pStr, LOG_TYPE::WARNING); \
	} \
	void LOG_FATAL(const char *pStr) \
	{ \
		CORE_REF->Log(pStr, LOG_TYPE::FATAL); \
	} \
	template <typename... Arguments> \
	void LOG_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::NORMAL, args...); \
	} \
	template <typename... Arguments> \
	void LOG_NORMAL_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::NORMAL, args...); \
	} \
	template <typename... Arguments> \
	void LOG_FATAL_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::FATAL, args...); \
	} \
	template <typename... Arguments> \
	void LOG_WARNING_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::WARNING, args...); \
	} \
	}

// encoding

#ifdef _WIN32
	typedef std::wstring mstring;
#else
	typedef std::string mstring;
#endif

string ConvertFromUtf16ToUtf8(const std::wstring& wstr);
std::wstring ConvertFromUtf8ToUtf16(const string& str);

#ifdef _WIN32
inline string NativeToUTF8(const std::wstring& wstr)
{
	return ConvertFromUtf16ToUtf8(wstr);
}
inline mstring UTF8ToNative(const string& str)
{
	return ConvertFromUtf8ToUtf16(str);
}
#else
#define NativeToUTF8(ARG) ARG
#define UTF8ToNative(ARG) ARG
#endif

std::list<string> get_file_content(const string& filename);
int is_relative(const char *pPath);
string make_absolute(const char *pRelDataPath, const char *pWorkingPath);

// lines manipulation
std::list<string> make_lines_list(const char **text);

template<class T>
const char** make_char_pp(const T& lines)
{
	char **ret = new char*[lines.size() + 1];

	memset(ret, 0, (lines.size() + 1) * sizeof(char*));

	int i = 0;
	for (auto it = lines.begin(); it != lines.end(); it++)
	{
		ret[i] = new char[it->size() + 1];
		strncpy(ret[i], it->c_str(), it->size());
		ret[i][it->size()] = '\0';
		i++;
	}

	return const_cast<const char**>(ret);
}

const char* make_char_p(const std::list<string>& lines);
void split_by_eol(const char **&text, int &num_lines, const string& str);
void delete_char_pp(const char **pText);

template<typename Out>
void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);


void look_at(Matrix4x4& Result, const Vector3 &eye, const Vector3 &center);


// math

inline bool Approximately(float l, float r)
{
	return std::abs(l - r) < EPSILON;
}

// Returns local X vector in world space
inline vec3 GetRightDirection(const mat4& ModelMat) { return vec3(ModelMat.Column(0)); }

// Returns local Y vector in world space
inline vec3 GetForwardDirection(const mat4& ModelMat) { return vec3(ModelMat.Column(1)); }

// Returns local -Z vector in world space
inline vec3 GetBackDirection(const mat4& ModelMat) { return -vec3(ModelMat.Column(2)); }

//
// Calculates Perspective matrix
// In right-handed coordinate system
// Camera: X - right, Y -top, Z - back
// Depth is in range [0, 1]
// fov - vertical (Y) in radians
// aspect - width / height
// 
// 	https://github.com/g-truc/glm/blob/0ceb2b755fb155d593854aefe3e45d416ce153a4/glm/ext/matrix_clip_space.inl perspectiveRH_ZO(T fovy, T aspect, T zNear, T zFar)
//
mat4 perspectiveRH_ZO(float fov, float aspect, float zNear, float zFar);


// random

int getRandomInt();

// subsystem
inline IResourceManager *getResourceManager(ICore *core)
{
	IResourceManager *ret;
	core->GetSubSystem((ISubSystem**)&ret, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	return ret;
}

inline ISceneManager *getSceneManager(ICore *core)
{
	ISceneManager *ret;
	core->GetSubSystem((ISubSystem**)&ret, SUBSYSTEM_TYPE::SCENE_MANAGER);
	return ret;
}

inline ICoreRender *getCoreRender(ICore *core)
{
	ICoreRender *ret;
	core->GetSubSystem((ISubSystem**)&ret, SUBSYSTEM_TYPE::CORE_RENDER);
	return ret;
}

// core render
int get_msaa_samples(INIT_FLAGS flags);
string msaa_to_string(int samples);

// texture formats
bool is_color_format(TEXTURE_FORMAT format);
bool is_compressed_format(TEXTURE_FORMAT format);

#define BASE_RESOURCE_HEADER \
private: \
	string _file; \
	int _refs = 0; \
public: \
	API GetReferences(int *refsOut) override; \
	API GetFile(OUT const char **file) override; \
	STDMETHODIMP_(ULONG) AddRef() override; \
	STDMETHODIMP_(ULONG) Release() override; \
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;


#define BASE_RESOURCE_IMPLEMENTATION(CLASS, CORE, REMOVE_RUNTIME_METHOD, REMOVE_SHARED_METHOD) \
 \
	API CLASS::GetReferences(int *refsOut) \
	{ \
		*refsOut = _refs; \
		return S_OK; \
	} \
	API CLASS::GetFile(OUT const char **file) \
	{ \
		*file = _file.c_str(); \
		return S_OK; \
	} \
	\
	STDMETHODIMP_(ULONG) CLASS::AddRef() \
	{ \
		_refs++; \
		return S_OK; \
	} \
	\
	STDMETHODIMP_(ULONG) CLASS::Release() \
	{ \
		_refs--; \
		if (_refs < 1) \
		{ \
			IResourceManager *irm = getResourceManager(CORE); \
			ResourceManager *rm = static_cast<ResourceManager*>(irm); \
			if (!_file.empty()) \
				rm->REMOVE_SHARED_METHOD(_file); \
			else \
				rm->REMOVE_RUNTIME_METHOD(this); \
			DEBUG_LOG_FORMATTED("delete %#010x", this); \
			delete this; \
		} \
		return S_OK;  \
	} \
 \
	STDMETHODIMP CLASS::QueryInterface(REFIID riid, void** ppv) \
	{  \
		*ppv = nullptr;  \
		return S_OK;  \
	}

#define RUNTIME_ONLY_RESOURCE_HEADER \
private: \
	int _refs = 0; \
public: \
	HRESULT GetReferences(int *refsOut) override; \
	STDMETHODIMP_(ULONG) AddRef() override; \
	STDMETHODIMP_(ULONG) Release() override; \
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;

#define RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(CLASS, CORE, REMOVE_RUNTIME_METHOD) \
	\
	HRESULT CLASS::GetReferences(int *refsOut) \
	{ \
		*refsOut = _refs; \
		return S_OK; \
	} \
	\
	STDMETHODIMP_(ULONG) CLASS::AddRef() \
	{ \
		_refs++; \
		return S_OK; \
	} \
	\
	STDMETHODIMP_(ULONG) CLASS::Release() \
	{ \
		_refs--; \
		if (_refs < 1) \
		{ \
			IResourceManager *irm = getResourceManager(CORE); \
			ResourceManager *rm = static_cast<ResourceManager*>(irm); \
			rm->REMOVE_RUNTIME_METHOD(this); \
			DEBUG_LOG_FORMATTED("delete %#010x", this); \
			delete this; \
		} \
		return S_OK;  \
	} \
	\
	STDMETHODIMP CLASS::QueryInterface(REFIID riid, void** ppv) \
	{  \
		*ppv = nullptr;  \
		return S_OK;  \
	}

#define SHARED_ONLY_RESOURCE_HEADER \
private: \
	string _file; \
	int _refs = 0; \
public: \
	API GetReferences(int *refsOut) override; \
	API GetFile(OUT const char **file) override; \
	API Reload() override; \
	STDMETHODIMP_(ULONG) AddRef() override; \
	STDMETHODIMP_(ULONG) Release() override; \
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;

#define SHARED_ONLY_RESOURCE_IMPLEMENTATION(CLASS, CORE, REMOVE_SHARED_METHOD) \
	\
	HRESULT CLASS::GetReferences(int *refsOut) \
	{ \
		*refsOut = _refs; \
		return S_OK; \
	} \
	\
	API CLASS::GetFile(OUT const char **file) \
	{ \
		*file = _file.c_str(); \
		return S_OK; \
	} \
	STDMETHODIMP_(ULONG) CLASS::AddRef() \
	{ \
		_refs++; \
		return S_OK; \
	} \
	\
	STDMETHODIMP_(ULONG) CLASS::Release() \
	{ \
		_refs--; \
		if (_refs < 1) \
		{ \
			IResourceManager *irm = getResourceManager(CORE); \
			ResourceManager *rm = static_cast<ResourceManager*>(irm); \
			rm->REMOVE_SHARED_METHOD(_file); \
			DEBUG_LOG_FORMATTED("delete %#010x", this); \
			delete this; \
		} \
		return S_OK;  \
	} \
	\
	STDMETHODIMP CLASS::QueryInterface(REFIID riid, void** ppv) \
	{  \
		*ppv = nullptr;  \
		return S_OK;  \
	}

//
enum class SHADER_TYPE
{
	SHADER_VERTEX,
	SHADER_GEOMETRY,
	SHADER_FRAGMENT
};

inline ICoreMesh *getCoreMesh(IMesh *mesh)
{
	ICoreMesh *m;
	mesh->GetCoreMesh(&m);
	return m;
}

inline ICoreTexture *getCoreTexture(ITexture *tex)
{
	ICoreTexture *t;
	tex->GetCoreTexture(&t);
	return t;
}

inline ICoreShader *getCoreShader(IShader *shader)
{
	ICoreShader *s;
	shader->GetCoreShader(&s);
	return s;
}

inline ICoreRenderTarget *getCoreRenderTarget(IRenderTarget *rt)
{
	ICoreRenderTarget *r;
	rt->GetCoreRenderTarget(&r);
	return r;
}

