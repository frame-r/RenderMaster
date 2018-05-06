#pragma once
#include "Engine.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


class Wnd
{
	HWND hwnd;
	void(*_main_loop)() { nullptr };

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	Wnd(void(*main_loop)());
	~Wnd();

	HWND *handle() { return &hwnd; }

	void CreateAndShow();
	void StartMainLoop();
	void Destroy();
	void GetDimension(uint& w, uint& h);
};

