#include "pch.h"
#include "console_window.h"
#include <map>

static std::map<string, std::function<void(const char** arguments, uint argumentsNum)>> _commands;

#define C_WND_EDIT_HEIGHT 16
#define C_WND_LISTBOX_HEIGHT 100

//void ConsoleWindow::_print_help()
//{
//	string txt;
//	for (auto &cmd : _commands)
//		txt += cmd.first + '\n';
//
//	LOG("Registered commands:");
//	LOG(txt.c_str());
//}

//void ConsoleWindow::_execute_command(const string& fullText)
//{
//	LOG(fullText.c_str());
//
//	vector<string> paths = split(string(fullText), ' ');
//	if (paths.size() <= 1)
//	{
//		string &name = paths[0];
//		ExecuteCommand(name.c_str(), nullptr, 0);
//	}
//	else
//	{
//		string &name = paths[0];
//		int arguments = (uint)paths.size();
//		const char **args = make_char_pp<vector<string>>(paths);
//
//		ExecuteCommand(name.c_str(), args, arguments);
//
//		delete_char_pp(args);
//	}
//}

LRESULT CALLBACK ConsoleWindow::_s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ConsoleWindow *this_ptr = (ConsoleWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

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
		MoveWindow(this_ptr->_hMemo, 0, 0, rect.right, rect.bottom - C_WND_EDIT_HEIGHT, true);
		MoveWindow(this_ptr->_hEdit, 0, rect.bottom - C_WND_EDIT_HEIGHT, rect.right, C_WND_EDIT_HEIGHT, true);
		//MoveWindow(this_ptr->_hListBox, 0, rect.bottom - C_WND_LISTBOX_HEIGHT - C_WND_EDIT_HEIGHT, rect.right, C_WND_LISTBOX_HEIGHT, true);
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

LRESULT CALLBACK ConsoleWindow::_s_WndEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ConsoleWindow *this_ptr = (ConsoleWindow *)GetWindowLongPtr(GetParent(hWnd), GWLP_USERDATA);

	//wchar_t wtmp[300];

	auto set_cursor_at_the_end = [=]()
	{
		DWORD TextSize;
		TextSize=GetWindowTextLength(this_ptr->_hEdit);
		SendMessage(this_ptr->_hEdit,EM_SETSEL,TextSize,TextSize);
	};	

	switch (message) 
	{
	case WM_KEYUP:
		switch (wParam)
		{
			case 192: //tilda
				if (this_ptr->_is_visible)
					this_ptr->Hide();
				SetWindowText(this_ptr->_hEdit, L"");
				break;

			//case 38: //up
			//{
			//	if (this_ptr->_completion_cmd_idx <= 0)
			//		break;

			//	if (this_ptr->_completion_cmd_idx == this_ptr->_completion_commands.size())
			//	{
			//		GetWindowText(this_ptr->_hEdit, wtmp, 300);
			//		this_ptr->_typed_command = NativeToUTF8(wtmp);
			//	}

			//	this_ptr->_completion_cmd_idx--;

			//	string mem_cmd = this_ptr->_completion_commands[this_ptr->_completion_cmd_idx];
			//	mstring wmem_cmd = UTF8ToNative(mem_cmd);
			//	SetWindowText(this_ptr->_hEdit, wmem_cmd.c_str());
			//	set_cursor_at_the_end();
			//}
				break;

			//case 40: // down
			//{
			//	if (this_ptr->_completion_cmd_idx > (int) (this_ptr->_completion_commands.size() - 1) || this_ptr->_completion_commands.empty())				
			//		break;

			//	if (this_ptr->_completion_cmd_idx == this_ptr->_completion_commands.size() - 1)
			//	{
			//		wstring wtyped = UTF8ToNative(this_ptr->_typed_command);
			//		this_ptr->_typed_command = "";
			//		SetWindowText(this_ptr->_hEdit, wtyped.c_str());
			//		set_cursor_at_the_end();

			//		
			//	} else
			//	{
			//		string mem_cmd = this_ptr->_completion_commands[this_ptr->_completion_cmd_idx + 1];
			//		mstring wmem_cmd = UTF8ToNative(mem_cmd);
			//		SetWindowText(this_ptr->_hEdit, wmem_cmd.c_str());
			//		set_cursor_at_the_end();
			//	}

			//	this_ptr->_completion_cmd_idx++;
			//}
				break;
		}
		break;

	case WM_CHAR:
		if (wParam == 96 /*tilda*/)
			break;

		// commnads
		//if (GetWindowTextLength(this_ptr->_hEdit) > 0)
		//{
		//	if (wParam == 9 /*tab*/)
		//	{
		//		GetWindowText(this_ptr->_hEdit, wtmp, 300);
		//		//this_ptr->_pConWindowEvent(this_ptr->_pConsole, CWE_COMPLETE_COMMAND, tmp);

		//		break;
		//	}
		//	else
		//		if (wParam == 13 /*return*/)
		//		{
		//			GetWindowText(this_ptr->_hEdit, wtmp, 300);
		//			SetWindowText(this_ptr->_hEdit, NULL);

		//			string tmp = NativeToUTF8(wtmp);

		//			this_ptr->_execute_command(tmp);
		//			this_ptr->_typed_command = "";
		//		
		//			break;
		//		}
		//}
		goto callDefWndPros;

	case WM_KEYDOWN:
		if (wParam == 38 /*up*/ || wParam == 40 /*down*/)
			break;
		else
			goto callDefWndPros;

	default:
		goto callDefWndPros;
	}

	return 0;

callDefWndPros:
	return CallWindowProc((WNDPROC)this_ptr->_pOldEditProc, hWnd, message, wParam, lParam);
}

WNDPROC ConsoleWindow::oldEditWndProc;

LRESULT CALLBACK ConsoleWindow::_s_EditProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if( (uMsg == WM_KEYUP) && (wParam == 192) )
    {
		ConsoleWindow *this_ptr = (ConsoleWindow *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (this_ptr->_is_visible)
			this_ptr->Hide();
		else
			this_ptr->Show();

        return 0;
    }
    return CallWindowProc(oldEditWndProc, hWnd, uMsg, wParam, lParam);
}

void ConsoleWindow::Init()
{
	HINSTANCE _hInst = GetModuleHandle(NULL);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)ConsoleWindow::_s_WndProc;
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
		1160, 200, 350, 500, h, NULL, _hInst, NULL);

	if (!_hWnd)
	{
		MessageBox(NULL, L"Failed to create console window!", L"Render Master Console", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		return;
	}

	SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	RECT client_rect;
	GetClientRect(_hWnd, &client_rect);
	_hMemo = CreateWindow(L"EDIT", L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL |
		ES_MULTILINE | ES_READONLY,
		0, 0, client_rect.right - client_rect.left, client_rect.bottom - client_rect.top, _hWnd, 0, 0, NULL);

	oldEditWndProc = (WNDPROC) SetWindowLongPtr(_hMemo, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)ConsoleWindow::_s_EditProc);
	SetWindowLongPtr(_hMemo, GWLP_USERDATA, (LONG_PTR)this);

	//SetWindowText(_hMemo, L"Console created\r\n");	

	LOGFONT LF = { 12, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Lucida Console" };
	_hFont = CreateFontIndirect(&LF);

	SendMessage(_hMemo, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));
	SendMessage(_hMemo, EM_LIMITTEXT, 200000, 0);

	_hEdit = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, _hWnd, 0, 0, NULL);	
	_pOldEditProc = (void *)SetWindowLongPtr(_hEdit, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)ConsoleWindow::_s_WndEditProc);
	SendMessage(_hEdit, WM_SETFONT, (WPARAM)_hFont, MAKELPARAM(TRUE, 0));

	//_hListBox = CreateWindow(L"LISTBOX", L"", WS_CHILD | WS_VISIBLE, 0, 400, 400, 400, _hMemo, 0, 0, NULL);
	//
	//SendMessage(_hListBox, LB_ADDSTRING, 0, (LPARAM)(LPSTR)L"hello");
	//SendMessage(_hListBox, LB_ADDSTRING, 0, (LPARAM)(LPSTR)L"hello");

	Show();
}

void ConsoleWindow::Destroy()
{
	DestroyWindow(_hWnd);
}

void ConsoleWindow::OutputTxt(const char* pStr)
{
	if (!_hMemo)
		return;

	int cur_l = GetWindowTextLength(_hMemo);

	SendMessage(_hMemo, EM_SETSEL, cur_l, cur_l);
	SendMessage(_hMemo, EM_REPLACESEL, false, (LPARAM)(std::wstring(ConvertFromUtf8ToUtf16(pStr)) + std::wstring(L"\r\n")).c_str());

	_prev_line_size = (int)strlen(pStr);
}

void ConsoleWindow::Show()
{
	_is_visible = 1;
	ShowWindow(_hWnd, SW_SHOW);
	UpdateWindow(_hWnd);
}

void ConsoleWindow::Hide()
{
	BringToFront();
	_is_visible = 0;
	ShowWindow(_hWnd, SW_HIDE);
}

void ConsoleWindow::BringToFront()
{
	SetActiveWindow(_hWnd);
}

//void ConsoleWindow::AddCommand(IConsoleCommand *pCommand)
//{
//	const char *pName;
//	pCommand->GetName(&pName);
//	string name = string(pName);
//	_commands.emplace(name, std::bind(&IConsoleCommand::Execute, pCommand, std::placeholders::_1, std::placeholders::_2));
//}

//void ConsoleWindow::ExecuteCommand(const char *name, const char **arguments, uint argumentsNum)
//{
//	auto it = _commands.find(string(name));
//
//	string commandFullText = string(name);
//	if (argumentsNum > 1)
//	{
//		for (size_t i = 1; i < argumentsNum; i++)
//			commandFullText += ' ' + string(*(arguments+i));
//	}
//	_completion_commands.push_back(commandFullText);
//	_completion_cmd_idx = (int)_completion_commands.size();
//
//	if (it == _commands.end())
//	{
//		LOG_WARNING_FORMATTED("Unknown command \'%s\'", name);
//		_print_help();
//		return;
//	}
//	std::function<void(const char** arguments, uint argumentsNum)> f = it->second;
//
//	f(arguments, argumentsNum);
//}
//
//
//void ConsoleWindow::GetCommands(OUT uint * number)
//{
//	*number = (uint) _commands.size();
//}
//
//void ConsoleWindow::GetCommand(OUT const char **name, uint idx)
//{
//	auto it = _commands.begin();
//	for(uint i = 0; i < idx; it++);
//	*name = it->first.c_str();
//}
