#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Wnd
{
	HWND hwnd;
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	Wnd();
	~Wnd();

	inline HWND *handle() { return &hwnd; }

	void CreateAndShow();
	void StartMainLoop();
	void Destroy();
};

