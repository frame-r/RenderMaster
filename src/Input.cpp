#include "Input.h"
#include "Core.h"
#include "Wnd.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

Input* Input::instance{nullptr};

void Input::_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void * pData)
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
}

Input::~Input()
{
}

API Input::IsKeyPressed(KEYBOARD_KEY_CODES key, int& isPressed)
{
	isPressed = _keys[(int)key] > 0;
	return S_OK;
}

API Input::IsMoisePressed(MOUSE_BUTTON type, int& isPressed)
{
	isPressed = _mouse[(int)type] > 0;
	return S_OK;
}

API Input::GetName(const char *&pName)
{
	pName = "Input";
	return S_OK;
}