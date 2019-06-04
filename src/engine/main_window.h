#pragma once
#include "common.h"

class MainWindow
{
	static MainWindow *thisPtr;
	HWND hwnd;
	void(*mainLoop)() {nullptr};
	Signal<WINDOW_MESSAGE, uint32, uint32, void*> onWindowEvent;
	int passiveMainLoop = 0;

	void invokeMesage(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK sWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:

	MainWindow(void(*main_loop)());
	~MainWindow();

	HWND *handle() { return &hwnd; }

	void Create();
	void StartMainLoop();
	void Destroy();
	void GetClientSize(int& w, int& h);
	void AddMessageCallback(WindowCallback c);
	void SetCaption(const wchar_t* text);
	void SetPassiveMainLoop(int value) { passiveMainLoop = value; }
};

