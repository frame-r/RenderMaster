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
