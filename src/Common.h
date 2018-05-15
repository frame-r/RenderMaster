#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "Engine.h"

#include <unordered_map>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <functional>
#include <assert.h>


using namespace RENDER_MASTER;

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

inline void standard_free_and_delete(IResource *pRes, std::function<void()> actually_delete_proc, ICore *_pCore)
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem**)&pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	uint refNum;
	pResMan->GetRefNumber(&refNum, pRes);

	if (refNum == 1)
	{
		pResMan->RemoveFromList(pRes);
		if (actually_delete_proc)
			actually_delete_proc();
	}
	else if (refNum > 1)
		pResMan->DecrementRef(pRes);
	else
	{
		pResMan->RemoveFromList(pRes);
		_pCore->Log("standard_free_and_delete: refNum == 0",LOG_TYPE::WARNING);
	}

	delete pRes;
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

bool is_relative(const char *pPath);
std::string make_absolute(const char *pRelDataPath, const char *pWorkingPath);
void split_by_eol(const char **&text, int &num_lines, const std::string& str);
void delete_char_pp(const char **pText);
void look_at(Matrix4x4& Result, const Vector3 &eye, const Vector3 &center);
