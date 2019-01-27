#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <stack>
#include <functional>
#include <cassert>
#include <memory>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <chrono>
#include <iterator>

#include <experimental/filesystem>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define INITGUID
#include <Unknwn.h>
#include <initguid.h>

#include "Engine.h"

#include <GLEW\glew.h>
#include <GLEW\wglew.h>

#ifdef DIRECTX_11_INCLUDED
#include <d3d11_2.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#endif

#include "Tree.h"
