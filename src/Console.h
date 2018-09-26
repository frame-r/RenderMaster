#pragma once
#include "Common.h"
#include "Events.h"

class Console : public IConsole
{
	HWND _hWnd  = 0;
	HWND _hMemo = 0;
	HWND _hEdit = 0;
	HFONT _hFont;
	int	_iX, _iY, _iWidth, _iHeight;
	int	_prev_line_size;
	int _is_visible = 0;
	static WNDPROC oldEditWndProc;
	unique_ptr<LogEvent> _evLog{new LogEvent};

	string fullLogPath;

	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	Console();
	~Console();

	void Init(bool createWindow);
	void Destroy();
	void OutputTxt(const char* pStr);
	void Show();
	int IsVisible() { return _is_visible; }
	void Hide();
	void BringToFront();


	virtual API Log(const char* text, LOG_TYPE type) override;
	virtual API AddCommand(IConsoleCommand *pCommand) override;
	virtual API RemoveCommand(IConsoleCommand *pCommand) override;

	// Events
	virtual API GetLogPrintedEv(OUT ILogEvent **pEvent) override;

	virtual API GetName(OUT const char **pName) override;

};

