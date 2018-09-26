#include "pch.h"
#include "Console.h"
#include "Core.h"

using namespace std;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


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
		//this_ptr->_bVisible = (wParam == TRUE);
		SetFocus(this_ptr->_hEdit);
	}
	break;

	case WM_CLOSE:
		this_ptr->Hide();
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

	case WM_KEYUP:
		if (wParam == 192) // ~
		{
			if (this_ptr->_is_visible)
				this_ptr->Hide();
			else
				this_ptr->Show();
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;

}

WNDPROC Console::oldEditWndProc;

LRESULT CALLBACK Console::_s_EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if( (uMsg == WM_KEYUP) && (wParam == 192) )
    {
		Console *this_ptr = (Console *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (this_ptr->_is_visible)
			this_ptr->Hide();
		else
			this_ptr->Show();

        return 0;
    }
    return CallWindowProc(oldEditWndProc, hWnd, uMsg, wParam, lParam);
}

Console::Console()
{
}

Console::~Console()
{
}

void Console::Init(bool createWindow)
{
	char *pDataPath;
	_pCore->GetDataDir(&pDataPath);
	fullLogPath = string(pDataPath) + "\\log.txt";

	if (!createWindow)
		return;

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

	_hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"ConsoleClass", L"Render Master Console",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX,
		1060, 300, 1000, 700, h, NULL, _hInst, NULL);

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

	oldEditWndProc = (WNDPROC) SetWindowLongPtr(_hMemo, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)Console::_s_EditProc);
	SetWindowLongPtr(_hMemo, GWLP_USERDATA, (LONG_PTR)this);

	SetWindowText(_hMemo, L"Console created\r\n");	

	LOGFONT LF = { 12, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Lucida Console" };
	_hFont = CreateFontIndirect(&LF);

	SendMessage(_hMemo, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));

	//_hEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, _hWnd, 0, 0, NULL);
	//_pOldEditProc = (void *)SetWindowLongPtr(_hEdit, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)CConsoleWindow::_s_WndEditProc);

	//SendMessage(_hEdit, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));

	SendMessage(_hMemo, EM_LIMITTEXT, 200000, 0);

	//ResetSizeAndPos();

	Show();
}

void Console::Destroy()
{
	DestroyWindow(_hWnd);
}

void Console::OutputTxt(const char* pStr)
{
	if (!_hMemo)
		return;

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

	_prev_line_size = (int)strlen(pStr);
}

void Console::Show()
{
	_is_visible = 1;
	ShowWindow(_hWnd, SW_SHOW);
	UpdateWindow(_hWnd);
}

void Console::Hide()
{
	BringToFront();
	_is_visible = 0;
	ShowWindow(_hWnd, SW_HIDE);
}

void Console::BringToFront()
{
	SetActiveWindow(_hWnd);
}

API Console::Log(const char *text, LOG_TYPE type)
{
	OutputTxt(text);

	std::ofstream log(fullLogPath, std::ios::out | std::ios::app);
	log << text << std::endl;
	log.close();

	std::cout << text << std::endl;

	_evLog->Fire(text, type);

	return S_OK;
}

API Console::AddCommand(IConsoleCommand * pCommand)
{
	return E_NOTIMPL;
}

API Console::RemoveCommand(IConsoleCommand * pCommand)
{
	return E_NOTIMPL;
}

API Console::GetLogPrintedEv(OUT ILogEvent **pEvent)
{
	*pEvent = _evLog.get();
	return S_OK;
}

API Console::GetName(OUT const char ** pName)
{
	*pName = "Console";
	return S_OK;
}
