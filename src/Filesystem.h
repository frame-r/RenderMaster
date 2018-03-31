#pragma once
#include "Common.h"
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;


class FileSystem : public IFileSystem
{
	std::string _dataPath;

public:

	FileSystem(const std::string& dataPath);

	API OpenFile(IFile*& pFile, const char* path, FILE_OPEN_MODE mode) override;
	API FileExist(const char *fullPath, int &exist) override;
	API GetName(const char *&pName) override;
};

class File : public IFile
{
	std::fstream _file;
	fs::path _fs_full_path;

public:

	File(const char *pName, std::ios_base::openmode& fileMode, const fs::path& fullPath);

	API Read(uint8 *pMem, uint bytes) override;
	API ReadStr(char *pStr, uint& str_bytes) override;
	API IsEndOfFile(int &eof) override;
	API Write(const uint8 *pMem, uint bytes) override;
	API WriteStr(const char *pStr) override;
	API FileSize(uint &size) override;
	API CloseAndFree() override;
};