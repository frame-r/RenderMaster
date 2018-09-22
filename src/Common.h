#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "Engine.h"

using namespace RENDER_MASTER;

using std::unique_ptr;

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

inline std::string ConvertFromUtf16ToUtf8(const std::wstring &wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

inline std::wstring ConvertFromUtf8ToUtf16(const std::string& str)
{
	std::wstring res;
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
	if (size > 0)
	{
		std::vector<wchar_t> buffer(size);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], size);
		res.assign(buffer.begin(), buffer.end() - 1);
	}
	return res;
}
std::list<std::string> get_file_content(const std::string& filename);
bool is_relative(const char *pPath);
std::string make_absolute(const char *pRelDataPath, const char *pWorkingPath);

// lines manipulation
std::list<std::string> make_lines_list(const char **text);
const char** make_char_pp(const std::list<std::string>& lines);
const char* make_char_p(const std::list<std::string>& lines);
void split_by_eol(const char **&text, int &num_lines, const std::string& str);
void delete_char_pp(const char **pText);


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

