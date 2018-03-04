#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Common.h"

class GLCoreRender : public ICoreRender
{
	HDC _hdc;
	HGLRC _hRC;
	HWND _hWnd;

public:

	GLCoreRender();
	~GLCoreRender();
	
	// ISubSystem
	API GetName(const char *&pTxt) override;

	API Init(WinHandle& handle) override;
	API Clear() override;
	API Free() override;
};

