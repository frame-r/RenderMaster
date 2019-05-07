#include "pch.h"
#include "console.h"
#include "core.h"
#include "filesystem.h"
#include "console_window.h"

#define LOG_FILE "\\log.txt"
#define MAX_CHARS 10000

Console::Console()
{
	tmpbuf = std::unique_ptr<char[]>(new char[MAX_CHARS]);
}

Console::~Console()
{
	tmpbuf.reset();
}

void Console::Init(bool createWindow)
{
	string fullPath = _core->GetWorkingPath() + LOG_FILE;

	FileSystem *fs = _core->GetFilesystem();

	if (fs->FileExist(fullPath.c_str()))
		fs->ClearFile(fullPath.c_str());

	if (createWindow)
	{
		window = new ConsoleWindow;
		window->Init();
	}
}

void Console::Free()
{
	if (window)
	{
		window->Destroy();
		delete window;
		window = nullptr;
	}
}

void Console::Show()
{
	if (window)
		window->Show();
}

void Console::Hide()
{
	if (window)
		window->Hide();
}

auto DLLEXPORT Console::Log(const char *msg, LOG_TYPE type) -> void
{
	string fullPath = _core->GetWorkingPath() + LOG_FILE;

	FileSystem *fs = _core->GetFilesystem();
	File f = fs->OpenFile(fullPath.c_str(), FILE_OPEN_MODE::APPEND);

	f.WriteStr(msg);
	f.WriteStr("\n");

	std::cout << msg << std::endl;

	if (window)
		window->OutputTxt(msg);

	onLog.Invoke(msg, type);
}

auto DLLEXPORT Console::AddCallback(ConsoleCallback c) -> void
{
	onLog.Add(c);
}

auto DLLEXPORT Console::RemoveCallback(ConsoleCallback c) -> void
{
	onLog.Erase(c);
}
