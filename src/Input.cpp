#include "Input.h"
#include "Core.h"
#include "Wnd.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

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
		_mouse[param1] = 1;
		break;

	case WINDOW_MESSAGE::MOUSE_UP:
		_mouse[param1] = 0;
		break;

	case WINDOW_MESSAGE::MOUSE_MOVE:
		_cursorX = param1;
		_cursorY = param2;
		break;

	default:
		break;
	}
}

void Input::_s_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void * pData)
{
	instance->_message_callback(type, param1, param2, pData);
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
