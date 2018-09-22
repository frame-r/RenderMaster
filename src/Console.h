#pragma once
#include "Common.h"

class Console
{
	HWND _hWnd, _hMemo, _hEdit;
	HFONT _hFont;
	int	_iX, _iY, _iWidth, _iHeight;
	bool _bVisible;
	int	_iPrevLineSize;
	
	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	Console();
	~Console();

	void Init(const WinHandle* handle);
	void Destroy();
	void OutputTxt(const char* pStr);
	void Show();
	void Hide();
	void BringToFront();
};

