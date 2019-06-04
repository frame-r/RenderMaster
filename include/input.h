#pragma once
#include "common.h"

class Input final
{
	static Input* instance;

	uint8 keys_[256]{};
	uint8 mouse_[3]{};
	int cursorX_{}, cursorY_{};
	vec2 oldPos_;
	vec2 mouseDeltaPos_;
	
	void clearMouse();
	void messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void sMessageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);

public:
	Input();
	~Input();

	// Internal API
	void Init();
	void Free();
	void Update();

public:
	auto DLLEXPORT IsKeyPressed(KEYBOARD_KEY_CODES key) -> bool;
	auto DLLEXPORT IsMoisePressed(MOUSE_BUTTON type) -> bool;
	auto DLLEXPORT GetMouseDeltaPos() -> vec2;
	auto DLLEXPORT GetMousePos() -> vec2;
};
