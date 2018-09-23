#pragma once
#include "Common.h"

class Wnd
{
	static Wnd *this_ptr;
	HWND hwnd;
	void(*_main_loop)() {nullptr};
	std::vector<WindowMessageCallback> _window_mesage_callbacks;

	void _invoke_mesage(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	LRESULT CALLBACK _wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	Wnd(void(*main_loop)());
	~Wnd();

	HWND *handle() { return &hwnd; }

	void CreateAndShow();
	void Show();
	void StartMainLoop();
	void Destroy();
	void GetDimension(uint& w, uint& h);
	void AddMessageCallback(WindowMessageCallback callback);
	void SetCaption(const wchar_t* text);
};

