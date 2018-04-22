#include "Console.h"

using namespace std;


LRESULT CALLBACK Console::_s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Console *this_ptr = (Console *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_MOVE:
	{
		this_ptr->_iX = LOWORD(lParam);
		this_ptr->_iY = HIWORD(lParam);
	}
	break;

	case WM_SHOWWINDOW:
	{
		this_ptr->_bVisible = (wParam == TRUE);
		SetFocus(this_ptr->_hEdit);
	}
	break;

	case WM_CLOSE:
		ShowWindow(this_ptr->_hWnd, SW_HIDE);
		break;

	case WM_SIZE:
	{
		this_ptr->_iWidth = LOWORD(lParam);
		this_ptr->_iHeight = HIWORD(lParam);
		RECT rect;
		GetClientRect(this_ptr->_hWnd, &rect);
		MoveWindow(this_ptr->_hMemo, 0, 0, rect.right, rect.bottom - 0, true);
	}
		break;

	case WM_DESTROY:
		DeleteObject(this_ptr->_hFont);
		break;

	case WM_GETMINMAXINFO:
	{
		POINT pt;
		pt.x = 300;
		pt.y = 30;
		((MINMAXINFO *)lParam)->ptMinTrackSize = pt;
	}
	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;

}

Console::Console()
{
}


Console::~Console()
{
}

void Console::Init(const WinHandle* handle)
{
	HINSTANCE _hInst = GetModuleHandle(NULL);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)Console::_s_WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = _hInst;
	wcex.hIcon = LoadIcon(_hInst, (LPCTSTR)"");
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"ConsoleClass";
	wcex.hIconSm = LoadIcon(_hInst, (LPCTSTR)"");

	if (FindAtom(L"ConsoleClass") == NULL && !RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"Failed to register console class!", L"DGLE Console", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return;
	}

	HWND h = 0;
	if (handle != nullptr)
		h = *handle;

	_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"ConsoleClass", L"Render Master Console",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX,
		1060, 300, 700, 700, h, NULL, _hInst, NULL);

	if (!_hWnd)
	{
		MessageBox(NULL, L"Failed to create console window!", L"Render Master Console", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return;
	}

	SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	RECT client_rect;
	GetClientRect(_hWnd, &client_rect);
	_hMemo = CreateWindow(L"EDIT", L"Render Master Console created...",
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		ES_MULTILINE | ES_READONLY,
		0, 0, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, _hWnd, 0, 0, NULL);

	SetWindowText(_hMemo, L"Console created\r\n");


	LOGFONT LF = { 12, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Lucida Console" };
	_hFont = CreateFontIndirect(&LF);

	SendMessage(_hMemo, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));

	_hEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, _hWnd, 0, 0, NULL);

	SetWindowLongPtr(_hEdit, GWLP_USERDATA, (LONG_PTR)this);

	//_pOldEditProc = (void *)SetWindowLongPtr(_hEdit, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)CConsoleWindow::_s_WndEditProc);

	SendMessage(_hEdit, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));

	//ResetSizeAndPos();

	ShowWindow(_hWnd, SW_SHOWNORMAL);
}

void Console::Destroy()
{//TODO
}

void Console::OutputTxt(const char* pStr)
{
	int cur_l = GetWindowTextLength(_hMemo);

	SendMessage(_hMemo, EM_SETSEL, cur_l, cur_l);
	//if (!newline)
	//	SendMessage(_hMemo, EM_REPLACESEL, false, (LPARAM)(/*std::wstring(L"\r\n") + */std::wstring(ConvertFromUtf8ToUtf16(pStr))).c_str());
	//else
		SendMessage(_hMemo, EM_REPLACESEL, false, (LPARAM)(std::wstring(ConvertFromUtf8ToUtf16(pStr)) + std::wstring(L"\r\n")).c_str());
	SendMessage(_hMemo, EM_SCROLL, SB_BOTTOM, 0);

	//else
	//{
	//	_bToPrevLineActive = bToPrevLine;

	//	if (_hThreadHandle && _hMemo == NULL)
	//		_strOnCreate += "\r\n"s + pcTxt;
	//	else
	//	{
	//		SendMessage(_hMemo, EM_SETSEL, cur_l, cur_l);
	//		SendMessage(_hMemo, EM_REPLACESEL, false, (LPARAM)("\r\n"s + pcTxt).c_str());
	//		SendMessage(_hMemo, EM_SCROLL, SB_BOTTOM, 0);
	//	}
	//}

	_iPrevLineSize = (int)strlen(pStr);
}

