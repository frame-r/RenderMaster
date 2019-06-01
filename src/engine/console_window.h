#pragma once
#include "common.h"

class ConsoleWindow
{
	HWND hWnd_{};
	HWND hMemo_ {};
	HWND hEdit_{};
	HWND hListBox_{};
	HFONT hFont_;
	int	x_, y_, width_, height_;
	int	prevLineSize_;
	int isVisible{};
	static WNDPROC oldEditWndProc;
	void *oldEditProc;
	string fullLogPath;

	static LRESULT CALLBACK _s_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK _s_WndEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	void Init();
	void Destroy();
	void OutputTxt(const char* pStr);
	void Show();
	int IsVisible() { return isVisible; }
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
