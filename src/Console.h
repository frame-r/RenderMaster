#pragma once
#include "Common.h"
#include "Events.h"

class Console : public IConsole
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
	unique_ptr<LogEvent> _evLog{new LogEvent};
	string fullLogPath;

	string _typed_command; //  save command that user types
	int _completion_cmd_idx = -1; // index of _completion_commands 0...vector.size()
	std::vector<string> _completion_commands;

	std::map<string, std::function<API(const char** arguments, uint argumentsNum)>> _commands;

	void _print_help();
	void _execute_command(const string& fullText);

	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_WndEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	Console();
	virtual ~Console();

	void Init(bool createWindow);
	void Destroy();
	void OutputTxt(const char* pStr);
	void Show();
	int IsVisible() { return _is_visible; }
	void Hide();
	void BringToFront();
	void addCommand(const string name, std::function<API(const char**, uint)>&& calback)
	{
		_commands.emplace(name, std::forward<std::function<API(const char**, uint)>>(calback));
	}

	virtual API Log(const char* text, LOG_TYPE type) override;
	virtual API AddCommand(IConsoleCommand *pCommand) override;
	virtual API ExecuteCommand(const char *name, const char** arguments, uint argumentsNum) override;
	virtual API GetLogPrintedEv(OUT ILogEvent **pEvent) override;
	virtual API GetCommands(OUT uint *number) override;
	virtual API GetCommand(OUT const char **name, uint idx) override;
	virtual API GetName(OUT const char **pName) override;
};

