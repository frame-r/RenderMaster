#include "pch.h"
#include "input.h"
#include "core.h"
#include "main_window.h"

#define LOWORD(_dw)				((WORD)(((DWORD_PTR)(_dw)) & 0xffff))
#define HIWORD(_dw)				((WORD)((((DWORD_PTR)(_dw)) >> 16) & 0xffff))
#define LODWORD(_qw)			((DWORD)(_qw))
#define HIDWORD(_qw)			((DWORD)(((_qw) >> 32) & 0xffffffff))
#define GET_X_LPARAM(lp)        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)        ((int)(short)HIWORD(lp))


Input* Input::instance{nullptr};

void Input::Update()
{
	mouseDeltaPos_.x = float(cursorX_) - oldPos_.x;
	mouseDeltaPos_.y = float(cursorY_) - oldPos_.y;
	oldPos_.x = (float)cursorX_;
	oldPos_.y = (float)cursorY_;
	//LOG_FORMATTED("cursorX_=%i cursorY_=%i mouseDeltaPos_(%f, %f) oldPos_(%f, %f)", cursorX_, cursorY_, mouseDeltaPos_.x, mouseDeltaPos_.y, oldPos_.x, oldPos_.y);
}

void Input::_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData)
{
	switch (type)
	{
	case WINDOW_MESSAGE::KEY_DOWN:
		keys_[param1] = 1;
		break;

	case WINDOW_MESSAGE::KEY_UP:
		keys_[param1] = 0;
		break;

	case WINDOW_MESSAGE::MOUSE_DOWN:
		cursorX_ = GET_X_LPARAM(param2); 
		cursorY_ = GET_Y_LPARAM(param2);
		oldPos_.x = float(cursorX_);
		oldPos_.y = float(cursorY_);
		mouse_[param1] = 1;
		break;

	case WINDOW_MESSAGE::MOUSE_UP:
		mouse_[param1] = 0;
		break;

	case WINDOW_MESSAGE::MOUSE_MOVE:
		//LOG_FORMATTED("x=%i y=%i", param1, param2);
		cursorX_ = param1;
		cursorY_ = param2;
		break;

	case WINDOW_MESSAGE::APPLICATION_ACTIVATED:
		clear_mouse();
		break;

	case WINDOW_MESSAGE::APPLICATION_DEACTIVATED:
		clear_mouse();
		break;

	default:
		break;
	}
}

void Input::_s_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void * pData)
{
	instance->_message_callback(type, param1, param2, pData);
}

void Input::Init()
{
	MainWindow *w = _core->GeWindow();

	if (w)
		w->AddMessageCallback(_s_message_callback);
}

void Input::Free()
{
}

void Input::clear_mouse()
{
	cursorX_ = 0;
	cursorY_ = 0;
	oldPos_ = vec2(0.0f);
	mouseDeltaPos_ = vec2(0.0f, 0.0f);
}

Input::Input()
{
	instance = this;
}

Input::~Input()
{
	instance = nullptr;
}

auto DLLEXPORT Input::IsKeyPressed(KEYBOARD_KEY_CODES key) -> bool
{
	return keys_[(int)key] > 0;
}

auto DLLEXPORT Input::IsMoisePressed(MOUSE_BUTTON type) -> bool
{
	return mouse_[(int)type] > 0;
}

auto DLLEXPORT Input::GetMouseDeltaPos() -> vec2
{
	return mouseDeltaPos_;
}

auto DLLEXPORT Input::GetMousePos() -> vec2
{
	return vec2((float)cursorX_, (float)cursorY_);
}

