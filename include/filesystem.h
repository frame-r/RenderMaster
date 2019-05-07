#pragma once
#include "common.h"
#include <filesystem>
#include <fstream>

class File;

class FileSystem final
{

public:
	// Internal API

	void Init();
	void Free();

public:
	auto DLLEXPORT FileExist(const char *path) -> bool;
	auto DLLEXPORT DirectoryExist(const char *path) -> bool;
	auto DLLEXPORT GetWorkingPath(const char *path) -> std::string;
	auto DLLEXPORT IsRelative(const char *path) -> bool;
	auto DLLEXPORT OpenFile(const char *path, FILE_OPEN_MODE mode = FILE_OPEN_MODE::WRITE) -> File;
	auto DLLEXPORT ClearFile(const char *path) -> void;
	auto DLLEXPORT GetPaths(const char *ext) -> std::vector<std::string>;
};

class File final
{
	std::fstream file_;
	std::filesystem::path fsPath_;

public:
	File(const std::ios_base::openmode& fileMode, const std::filesystem::path& path);
	~File();
	File& operator=(const File&) = delete;
	File(const File&) = delete;
	File& operator=(const File&&);
	File(const File&&);

	auto DLLEXPORT Read(uint8 *pMem, size_t bytes) -> void;
	auto DLLEXPORT Write(const uint8 *pMem, size_t bytes) -> void;
	auto DLLEXPORT WriteStr(const char *str) -> void;
	auto DLLEXPORT FileSize() -> size_t;
};

