#pragma once
#include "Common.h"

class Input : public IInput
{
	static Input* instance;

	uint8 _keys[256]{};
	uint8 _mouse[3]{};
	int _cursorX{}, _cursorY{};

	void _message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void _s_message_callback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);

public:

	Input();
	~Input();

	API IsKeyPressed(OUT int *isPressed, KEYBOARD_KEY_CODES key) override;
	API IsMoisePressed(OUT int *isPressed, MOUSE_BUTTON type) override;
	API GetName(OUT const char **pName) override;
};