#include "pch.h"
#include "Wnd.h"
#include "Core.h"

#define WIN32_LEAN_AND_MEAN
#include <Windowsx.h>

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

#define UPDATE_TIMER_ID 1

KEYBOARD_KEY_CODES ASCIIKeyToEngKey(unsigned char key)
{
	switch (key)
	{
	case 27: return KEYBOARD_KEY_CODES::KEY_ESCAPE;
	case 9: return KEYBOARD_KEY_CODES::KEY_TAB;
	case 192: return KEYBOARD_KEY_CODES::KEY_GRAVE;
	case 20: return KEYBOARD_KEY_CODES::KEY_CAPSLOCK;
	case 8: return KEYBOARD_KEY_CODES::KEY_BACKSPACE;
	case 13: return KEYBOARD_KEY_CODES::KEY_RETURN;
	case 32: return KEYBOARD_KEY_CODES::KEY_SPACE;
	case 191: return KEYBOARD_KEY_CODES::KEY_SLASH;
	case 220: return KEYBOARD_KEY_CODES::KEY_BACKSLASH;

	case 44: return KEYBOARD_KEY_CODES::KEY_SYSRQ;
	case 145: return KEYBOARD_KEY_CODES::KEY_SCROLL;
	case 19: return KEYBOARD_KEY_CODES::KEY_PAUSE;

	case 45: return KEYBOARD_KEY_CODES::KEY_INSERT;
	case 46: return KEYBOARD_KEY_CODES::KEY_DELETE;
	case 36: return KEYBOARD_KEY_CODES::KEY_HOME;
	case 35: return KEYBOARD_KEY_CODES::KEY_END;
	case 33: return KEYBOARD_KEY_CODES::KEY_PGUP;
	case 34: return KEYBOARD_KEY_CODES::KEY_PGDN;

	case 16: return KEYBOARD_KEY_CODES::KEY_LSHIFT;
	case 18: return KEYBOARD_KEY_CODES::KEY_LALT;
	case 17: return KEYBOARD_KEY_CODES::KEY_LCONTROL;

	case 38: return KEYBOARD_KEY_CODES::KEY_UP;
	case 39: return KEYBOARD_KEY_CODES::KEY_RIGHT;
	case 37: return KEYBOARD_KEY_CODES::KEY_LEFT;
	case 40: return KEYBOARD_KEY_CODES::KEY_DOWN;

	case 48: return KEYBOARD_KEY_CODES::KEY_0;
	case 49: return KEYBOARD_KEY_CODES::KEY_1;
	case 50: return KEYBOARD_KEY_CODES::KEY_2;
	case 51: return KEYBOARD_KEY_CODES::KEY_3;
	case 52: return KEYBOARD_KEY_CODES::KEY_4;
	case 53: return KEYBOARD_KEY_CODES::KEY_5;
	case 54: return KEYBOARD_KEY_CODES::KEY_6;
	case 55: return KEYBOARD_KEY_CODES::KEY_7;
	case 56: return KEYBOARD_KEY_CODES::KEY_8;
	case 57: return KEYBOARD_KEY_CODES::KEY_9;

	case 112: return KEYBOARD_KEY_CODES::KEY_F1;
	case 113: return KEYBOARD_KEY_CODES::KEY_F2;
	case 114: return KEYBOARD_KEY_CODES::KEY_F3;
	case 115: return KEYBOARD_KEY_CODES::KEY_F4;
	case 116: return KEYBOARD_KEY_CODES::KEY_F5;
	case 117: return KEYBOARD_KEY_CODES::KEY_F6;
	case 118: return KEYBOARD_KEY_CODES::KEY_F7;
	case 119: return KEYBOARD_KEY_CODES::KEY_F8;
	case 120: return KEYBOARD_KEY_CODES::KEY_F9;
	case 121: return KEYBOARD_KEY_CODES::KEY_F10;
	case 122: return KEYBOARD_KEY_CODES::KEY_F11;
	case 123: return KEYBOARD_KEY_CODES::KEY_F12;

	case 81: return KEYBOARD_KEY_CODES::KEY_Q;
	case 87: return KEYBOARD_KEY_CODES::KEY_W;
	case 69: return KEYBOARD_KEY_CODES::KEY_E;
	case 82: return KEYBOARD_KEY_CODES::KEY_R;
	case 84: return KEYBOARD_KEY_CODES::KEY_T;
	case 89: return KEYBOARD_KEY_CODES::KEY_Y;
	case 85: return KEYBOARD_KEY_CODES::KEY_U;
	case 73: return KEYBOARD_KEY_CODES::KEY_I;
	case 79: return KEYBOARD_KEY_CODES::KEY_O;
	case 80: return KEYBOARD_KEY_CODES::KEY_P;
	case 65: return KEYBOARD_KEY_CODES::KEY_A;
	case 83: return KEYBOARD_KEY_CODES::KEY_S;
	case 68: return KEYBOARD_KEY_CODES::KEY_D;
	case 70: return KEYBOARD_KEY_CODES::KEY_F;
	case 71: return KEYBOARD_KEY_CODES::KEY_G;
	case 72: return KEYBOARD_KEY_CODES::KEY_H;
	case 74: return KEYBOARD_KEY_CODES::KEY_J;
	case 75: return KEYBOARD_KEY_CODES::KEY_K;
	case 76: return KEYBOARD_KEY_CODES::KEY_L;
	case 90: return KEYBOARD_KEY_CODES::KEY_Z;
	case 88: return KEYBOARD_KEY_CODES::KEY_X;
	case 67: return KEYBOARD_KEY_CODES::KEY_C;
	case 86: return KEYBOARD_KEY_CODES::KEY_V;
	case 66: return KEYBOARD_KEY_CODES::KEY_B;
	case 78: return KEYBOARD_KEY_CODES::KEY_N;
	case 77: return KEYBOARD_KEY_CODES::KEY_M;

	case 189: return KEYBOARD_KEY_CODES::KEY_MINUS;
	case 187: return KEYBOARD_KEY_CODES::KEY_PLUS;
	case 219: return KEYBOARD_KEY_CODES::KEY_LBRACKET;
	case 221: return KEYBOARD_KEY_CODES::KEY_RBRACKET;

	case 186: return KEYBOARD_KEY_CODES::KEY_SEMICOLON;
	case 222: return KEYBOARD_KEY_CODES::KEY_APOSTROPHE;

	case 188: return KEYBOARD_KEY_CODES::KEY_COMMA;
	case 190: return KEYBOARD_KEY_CODES::KEY_PERIOD;

	case 96: return KEYBOARD_KEY_CODES::KEY_NUMPAD0;
	case 97: return KEYBOARD_KEY_CODES::KEY_NUMPAD1;
	case 98: return KEYBOARD_KEY_CODES::KEY_NUMPAD2;
	case 99: return KEYBOARD_KEY_CODES::KEY_NUMPAD3;
	case 100: return KEYBOARD_KEY_CODES::KEY_NUMPAD4;
	case 101: return KEYBOARD_KEY_CODES::KEY_NUMPAD5;
	case 102: return KEYBOARD_KEY_CODES::KEY_NUMPAD6;
	case 103: return KEYBOARD_KEY_CODES::KEY_NUMPAD7;
	case 104: return KEYBOARD_KEY_CODES::KEY_NUMPAD8;
	case 105: return KEYBOARD_KEY_CODES::KEY_NUMPAD9;
	case 110: return KEYBOARD_KEY_CODES::KEY_NUMPADPERIOD;
	case 106: return KEYBOARD_KEY_CODES::KEY_NUMPADSTAR;
	case 107: return KEYBOARD_KEY_CODES::KEY_NUMPADPLUS;
	case 109: return KEYBOARD_KEY_CODES::KEY_NUMPADMINUS;
	case 111: return KEYBOARD_KEY_CODES::KEY_NUMPADSLASH;
	case 144: return KEYBOARD_KEY_CODES::KEY_NUMLOCK;
	default: return KEYBOARD_KEY_CODES::KEY_UNKNOWN;
	}
}

Wnd *Wnd::this_ptr{nullptr};

Wnd::Wnd(void(*main_loop)())
{
	this_ptr = this;
	_main_loop = main_loop;
}

Wnd::~Wnd()
{
}

void Wnd::_invoke_mesage(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	for (auto c : _window_mesage_callbacks)
		c(type, param1, param2, pData);
}


LRESULT Wnd::_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	if (message == WM_CLOSE)
	{
		PostQuitMessage(0);
		_invoke_mesage(WINDOW_MESSAGE::WINDOW_CLOSE, 0, 0, nullptr);
		return 0;
	}

	switch (message)
	{
	//case WM_SIZING:
	//{
	//	RECT r{};
	//	GetClientRect(hWnd, &r);
	//	_invoke_mesage(WINDOW_MESSAGE::SIZE, r.right - r.left, r.bottom - r.top, (void*)lParam);
	//}

	case WM_SIZE:
	{
		switch (wParam)
		{
		case SIZE_MAXHIDE:
			//LOG("WM_SIZE::SIZE_MAXHIDE");
			break;
		case SIZE_MAXIMIZED:
			//LOG("WM_SIZE::SIZE_MAXIMIZED");
			break;
		case SIZE_MAXSHOW:
			//LOG("WM_SIZE::SIZE_MAXSHOW");
			break;
		case SIZE_MINIMIZED:
			//LOG("WM_SIZE::SIZE_MINIMIZED");
			_invoke_mesage(WINDOW_MESSAGE::WINDOW_MINIMIZED, 0, 0, nullptr);
			break;
		case SIZE_RESTORED:
			//LOG("WM_SIZE::SIZE_RESTORED");
			_invoke_mesage(WINDOW_MESSAGE::WINDOW_UNMINIMIZED, 0, 0, nullptr);
			break;
		}
		_invoke_mesage(WINDOW_MESSAGE::SIZE, LOWORD(lParam), HIWORD(lParam), (void*)lParam);
	}
	break;


	case WM_KEYDOWN:
		if (lParam & 0x00100000)
		{
			if (wParam == 16)
				_invoke_mesage(WINDOW_MESSAGE::KEY_DOWN, (uint32)KEYBOARD_KEY_CODES::KEY_RSHIFT, 0, nullptr);
			else
				if (wParam == 17)
					_invoke_mesage(WINDOW_MESSAGE::KEY_DOWN, (uint32)KEYBOARD_KEY_CODES::KEY_RCONTROL, 0, nullptr);
				else
					_invoke_mesage(WINDOW_MESSAGE::KEY_DOWN, (uint32)ASCIIKeyToEngKey((unsigned char)wParam), 0, nullptr);
		}
		else
			_invoke_mesage(WINDOW_MESSAGE::KEY_DOWN, (uint32)ASCIIKeyToEngKey((unsigned char)wParam), 0, nullptr);
		break;

	case WM_KEYUP:
		if (lParam & 0x00100000)
		{
			if (wParam == 16)
				_invoke_mesage(WINDOW_MESSAGE::KEY_UP, (uint32)KEYBOARD_KEY_CODES::KEY_RSHIFT, 0, nullptr);
			else
				if (wParam == 17)
					_invoke_mesage(WINDOW_MESSAGE::KEY_UP, (uint32)KEYBOARD_KEY_CODES::KEY_RCONTROL, 0, nullptr);
				else
					_invoke_mesage(WINDOW_MESSAGE::KEY_UP, (uint32)ASCIIKeyToEngKey((unsigned char)wParam), 0, nullptr);
		}
		else
			_invoke_mesage(WINDOW_MESSAGE::KEY_UP, (uint32)ASCIIKeyToEngKey((unsigned char)wParam), 0, nullptr);
		break;

	case WM_MOUSEMOVE:
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_MOVE, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), nullptr);
		break;

	case WM_LBUTTONDOWN:
		//LOG("WM_LBUTTONDOWN");
		SetCapture(hWnd);
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_DOWN, 0, 0, nullptr);
		break;

	case WM_RBUTTONDOWN:
		
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_DOWN, 1, 0, nullptr);
		break;

	case WM_MBUTTONDOWN:
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_DOWN, 2, 0, nullptr);
		break;

	case WM_LBUTTONUP:
		//LOG("WM_LBUTTONUP");
		ReleaseCapture();
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_UP, 0, 0, nullptr);
		break;

	case WM_RBUTTONUP:
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_UP, 1, 0, nullptr);
		break;

	case WM_MBUTTONUP:
		_invoke_mesage(WINDOW_MESSAGE::MOUSE_UP, 2, 0, nullptr);
		break;

	case WM_MOUSELEAVE:
		//LOG("WM_MOUSELEAVE");
		break;

	case WM_MOUSEHOVER:
		//LOG("WM_MOUSEHOVER");
		break;

	case WM_ACTIVATE:
	{
		int fActive = LOWORD(wParam);
		//if (fActive) LOG("WM_ACTIVATE true");
		//else LOG("WM_ACTIVATE false");
		if (fActive)
			_invoke_mesage(WINDOW_MESSAGE::WINDOW_ACTIVATED, 0, 0, nullptr);
		else
			_invoke_mesage(WINDOW_MESSAGE::WINDOW_DEACTIVATED, 0, 0, nullptr);
	}
	break;

	case WM_ACTIVATEAPP:
	{
		int fActive = LOWORD(wParam);
		//if (fActive) LOG("WM_ACTIVATEAPP true");
		//else LOG("WM_ACTIVATEAPP false");
	}
	break;

	case WM_ENTERSIZEMOVE:
		//_invoke_mesage(WINDOW_MESSAGE::WINDOW_DEACTIVATED, 0, 0, nullptr);
		SetTimer(hWnd, UPDATE_TIMER_ID, USER_TIMER_MINIMUM, NULL);
		break;

	case WM_EXITSIZEMOVE:
		//_invoke_mesage(WINDOW_MESSAGE::WINDOW_ACTIVATED, 0, 0, nullptr);
		KillTimer(hWnd, UPDATE_TIMER_ID);
		break;

	case WM_TIMER:
		if (wParam == UPDATE_TIMER_ID)
		{
			//LOG("WM_TIMER redraw");
			_invoke_mesage(WINDOW_MESSAGE::WINDOW_REDRAW, 0, 0, nullptr);
		}
		break;

	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;



	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT CALLBACK Wnd::_s_wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return this_ptr->_wndProc(hWnd, message, wParam, lParam);
}

void Wnd::CreateAndShow()
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = _s_wndProc;
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

}

void Wnd::Show()
{
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

void Wnd::AddMessageCallback(WindowMessageCallback callback)
{
	_window_mesage_callbacks.push_back(callback);
}

void Wnd::SetCaption(const wchar_t* text)
{
	SetWindowText(hwnd, text);
}
