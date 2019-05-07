#pragma once
#include "common.h"

class ConsoleWindow
{
	HWND _hWnd  = 0;
	HWND _hMemo = 0;
	HWND _hEdit = 0;
	HWND _hListBox = 0;

	HFONT _hFont;
	int	_iX, _iY, _iWidth, _iHeight;
	int	_prev_line_size;
	int _is_visible = 0;
	static WNDPROC oldEditWndProc;
	void *_pOldEditProc;
	string fullLogPath;

	//string _typed_command; //  save command that user types
	//int _completion_cmd_idx = -1; // index of _completion_commands 0...vector.size()
	//std::vector<string> _completion_commands;

	//void _print_help();
	//void _execute_command(const string& fullText);

	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_WndEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	void Init();
	void Destroy();
	void OutputTxt(const char* pStr);
	void Show();
	int IsVisible() { return _is_visible; }
	void Hide();
	void BringToFront();
	//void addCommand(const string name, std::function<void(const char**, uint)>&& calback)
	//{
	//	_commands.emplace(name, std::forward<std::function<void(const char**, uint)>>(calback));
	//}

	//virtual void AddCommand(IConsoleCommand *pCommand) override;
	//virtual void ExecuteCommand(const char *name, const char** arguments, uint argumentsNum) override;
	//virtual void GetCommands(OUT uint *number) override;
	//virtual void GetCommand(OUT const char **name, uint idx) override;

};
