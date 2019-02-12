#pragma once
#include "Common.h"
#include <filesystem>

namespace fs = std::filesystem;


class FileSystem final: public IFileSystem
{
	string _dataPath;

public:
	FileSystem();
	virtual ~FileSystem(){}

	void Init(const string& dataPath);
	vector<string> GetAllFiles();

	API_RESULT OpenFile(OUT IFile **pFile, const char* path, FILE_OPEN_MODE mode) override;
	API_RESULT FileExist(const char *fullPath, OUT int *exist) override;
	API_RESULT DirectoryExist(const char *fullPath, OUT int *exist) override;
	API_RESULT GetName(OUT const char **pName) override;
};


class File final : public IFile
{
	std::fstream _file;
	fs::path _fsPath;

public:
	File(const std::ios_base::openmode& fileMode, const fs::path& path);

	API_RESULT Read(OUT uint8 *pMem, uint bytes) override;
	API_RESULT ReadStr(OUT char *pStr, OUT uint *bytes) override;
	API_RESULT IsEndOfFile(OUT int *eof) override;
	API_RESULT Write(const uint8 *pMem, uint bytes) override;
	API_RESULT WriteStr(const char *pStr) override;
	API_RESULT FileSize(OUT uint *size) override;
	API_RESULT CloseAndFree() override;
};
