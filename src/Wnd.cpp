#include "Wnd.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>


Wnd::Wnd(void(*main_loop)())
{
	_main_loop = main_loop;
}

Wnd::~Wnd()
{
}

LRESULT CALLBACK Wnd::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CLOSE)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Wnd::CreateAndShow()
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"simple class name";
	wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	assert(RegisterClassExW(&wcex) != FALSE);

	hwnd = CreateWindow(TEXT("Simple class name"), TEXT("Test"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
}

void Wnd::StartMainLoop()
{
	MSG msg;

	while (true)
	{
		if (_main_loop != nullptr)
		{

			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				_main_loop();
			}
		}
		else
		{
			if (GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else // WM_QUIT
				break;
		}
	}
	
}

void Wnd::Destroy()
{
	DestroyWindow(hwnd);
}

void Wnd::GetDimension(uint& w, uint& h)
{
	RECT rect;

	GetClientRect(hwnd, &rect);
	
	w = rect.right - rect.left;
	h = rect.bottom - rect.top;
}
