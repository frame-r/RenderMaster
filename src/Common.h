#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "Engine.h"
#include <string>
#include <functional>

using namespace RENDER_MASTER;

template<typename Char>
std::basic_string<Char> ToLowerCase(std::basic_string<Char> str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

#ifdef _DEBUG
#define DEFINE_DEBUG_LOG_HELPERS(CORE_REF) \
	template <typename... Arguments> \
	void DEBUG_LOG(const char *pStr, LOG_TYPE type, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, type, args...); \
	}
#else
#define DEFINE_DEBUG_LOG_HELPERS(CORE_REF) \
	template <typename... Arguments> \
	void DEBUG_LOG(const char *pStr, LOG_TYPE type, Arguments ... args) \
	{ \
	}
#endif

#define DEFINE_LOG_HELPERS(CORE_REF) \
	void LOG(const char *pStr, LOG_TYPE type = LOG_TYPE::LT_NORMAL) \
	{ \
		CORE_REF->Log(pStr, type); \
	} \
	void LOG_FATAL(const char *pStr) \
	{ \
		CORE_REF->Log(pStr, LOG_TYPE::LT_FATAL); \
	} \
	template <typename... Arguments> \
	void LOG_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::LT_NORMAL, args...); \
	} \
	template <typename... Arguments> \
	void LOG_NORMAL_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::LT_NORMAL, args...); \
	} \
	template <typename... Arguments> \
	void LOG_FATAL_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::LT_FATAL, args...); \
	} \
	template <typename... Arguments> \
	void LOG_WARNING_FORMATTED(const char *pStr, Arguments ... args) \
	{ \
		CORE_REF->LogFormatted(pStr, LOG_TYPE::LT_WARNING, args...); \
	}


