#pragma once
#include "Common.h"

class Console
{
	HWND _hWnd, _hMemo, _hEdit;
	HFONT _hFont;
	int	_iX, _iY, _iWidth, _iHeight;
	int	_prev_line_size;
	int _is_visible = 0;
	static WNDPROC oldEditWndProc;

	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	Console();
	~Console();

	void Init(const WinHandle* handle);
	void Destroy();
	void OutputTxt(const char* pStr);
	void Show();
	int IsVisible() { return _is_visible; }
	void Hide();
	void BringToFront();
};

