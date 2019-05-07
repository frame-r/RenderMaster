#pragma once
#include "common.h"

class ConsoleWindow;

class Console final
{
	Signal<const char*, LOG_TYPE> onLog;
	std::unique_ptr<char[]> tmpbuf;

	ConsoleWindow *window{nullptr};

public:
	// Internal API
	Console();
	~Console();

	void Init(bool createWindow);
	void Free();
	void Show();
	void Hide();

	template <typename... Arguments>
	void Log(const char *pStr, LOG_TYPE type, Arguments ...args)
	{
		sprintf(tmpbuf.get(), pStr, args...);
		Log(tmpbuf.get(), type);
	}

public:

	auto DLLEXPORT Log(const char* msg, LOG_TYPE type = LOG_TYPE::NORMAL) -> void;
	auto DLLEXPORT AddCallback(ConsoleCallback c) -> void;
	auto DLLEXPORT RemoveCallback(ConsoleCallback c) -> void;
};
