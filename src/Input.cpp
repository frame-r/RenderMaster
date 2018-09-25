#include "pch.h"
#include "Input.h"
#include "Core.h"
#include "Wnd.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

#define LOWORD(_dw)				((WORD)(((DWORD_PTR)(_dw)) & 0xffff))
#define HIWORD(_dw)				((WORD)((((DWORD_PTR)(_dw)) >> 16) & 0xffff))
#define LODWORD(_qw)			((DWORD)(_qw))
#define HIDWORD(_qw)			((DWORD)(((_qw) >> 32) & 0xffffffff))
#define GET_X_LPARAM(lp)        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)        ((int)(short)HIWORD(lp))


Input* Input::instance{nullptr};

void Input::_update()
{
	_mouseDeltaPos.x = float(_cursorX) - _oldPos.x;
	_mouseDeltaPos.y = float(_cursorY) - _oldPos.y;
	_oldPos.x = (float)_cursorX;
	_oldPos.y = (float)_cursorY;
	//LOG_FORMATTED("_cursorX=%i _cursorY=%i _mouseDeltaPos(%f, %f) _oldPos(%f, %f)", _cursorX, _cursorY, _mouseDeltaPos.x, _mouseDeltaPos.y, _oldPos.x, _oldPos.y);
}

void Input::_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData)
{
	switch (type)
	{
	case WINDOW_MESSAGE::KEY_DOWN:
		_keys[param1] = 1;
		break;

	case WINDOW_MESSAGE::KEY_UP:
		_keys[param1] = 0;
		break;

	case WINDOW_MESSAGE::MOUSE_DOWN:
		_cursorX = GET_X_LPARAM(param2); 
		_cursorY = GET_Y_LPARAM(param2);
		_oldPos.x = float(_cursorX);
		_oldPos.y = float(_cursorY);
		_mouse[param1] = 1;
		break;

	case WINDOW_MESSAGE::MOUSE_UP:
		_mouse[param1] = 0;
		break;

	case WINDOW_MESSAGE::MOUSE_MOVE:
		//LOG_FORMATTED("x=%i y=%i", param1, param2);
		_cursorX = param1;
		_cursorY = param2;
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

void Input::clear_mouse()
{
	_cursorX = 0;
	_cursorY = 0;
	_oldPos = vec2(0.0f);
	_mouseDeltaPos = vec2(0.0f, 0.0f);
}

Input::Input()
{
	instance = this;

	if (_pCore->MainWindow())
		_pCore->MainWindow()->AddMessageCallback(_s_message_callback);

	_pCore->AddUpdateCallback(std::bind(&Input::_update, this));
}

Input::~Input()
{
}

API Input::IsKeyPressed(OUT int *isPressed, KEYBOARD_KEY_CODES key)
{
	*isPressed = _keys[(int)key] > 0;
	return S_OK;
}

API Input::IsMoisePressed(OUT int *isPressed, MOUSE_BUTTON type)
{
	*isPressed = _mouse[(int)type] > 0;
	return S_OK;
}

API Input::GetMouseDeltaPos(OUT vec2 *dPos)
{
	*dPos = _mouseDeltaPos;
	return S_OK;
}

API Input::GetName(OUT const char **pName)
{
	*pName = "Input";
	return S_OK;
}
