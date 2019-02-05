#include "Pch.h"
#include "Filesystem.h"
#include "Core.h"

using namespace std;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

FileSystem::FileSystem()
{
}

void FileSystem::Init(const string& dataPath)
{
	_dataPath = dataPath;
}

API_RESULT FileSystem::OpenFile(OUT IFile **pFile, const char* pPath, FILE_OPEN_MODE mode)
{
	const bool read = (mode & FILE_OPEN_MODE::READ) == FILE_OPEN_MODE::READ;
	const bool write = (mode & FILE_OPEN_MODE::WRITE) == FILE_OPEN_MODE::WRITE;
	
	fs::path fsPath = fs::u8path(pPath);

	if (fsPath.is_relative())
	{
		fs::path fsDataPath = fs::u8path(_dataPath);
		fsPath = fsDataPath / fsPath;
	}

	if (!exists(fsPath) && read)
	{
		LOG_FATAL("file isn't exist");
		pFile = nullptr;
		return S_FALSE;
	}

	ios_base::openmode cpp_mode;

	if (read)
		cpp_mode = ofstream::in;
	else if (write)
		cpp_mode = ofstream::out;
	else
	{
		LOG_FATAL("unkonwn mode!");
		pFile = nullptr;
		return S_FALSE;
	}

	if ((int)(FILE_OPEN_MODE::APPEND & mode))
		cpp_mode |= ofstream::app;

	if ((int)(mode & FILE_OPEN_MODE::BINARY))
		cpp_mode |= ofstream::binary;

	File *_pFile = new File(cpp_mode, fsPath);
	*pFile = _pFile;

	return S_OK;

}

API_RESULT FileSystem::FileExist(const char* fullPath, OUT int *exist)
{
	wstring wpPath = ConvertFromUtf8ToUtf16(fullPath);

	fs::path wfsPath(wpPath);
	fs::path wfsFullPath(wpPath);

	if (wfsPath.is_relative())
	{
		wstring wdataPath = ConvertFromUtf8ToUtf16(_dataPath);
		wfsFullPath = wstring(wdataPath + L'\\' + wpPath);
	}

	*exist = exists(wfsFullPath);

	return S_OK;
}

API_RESULT FileSystem::DirectoryExist(const char* fullPath, int* exist)
{
	wstring wpPath = ConvertFromUtf8ToUtf16(fullPath);
	fs::path wfsPath(wpPath);
	*exist = fs::exists(wfsPath);
	return S_OK;
}

API_RESULT FileSystem::GetName(OUT const char **pName)
{
	*pName = "FileSystem";
	return S_OK;
}

File::File(const ios_base::openmode& cpp_mode, const fs::path& path)
{
	_fsPath = path;	
	mstring mPath = UTF8ToNative(path.u8string());
	_file.open(mPath, cpp_mode);
}

API_RESULT File::Read(OUT uint8 *pMem, uint bytes)
{
	_file.read((char *)pMem, bytes);

	if (!_file)
	{
		LOG_WARNING_FORMATTED("File::Read(): read only %i bytes", _file.gcount());
		_file.clear();
		return S_FALSE;
	}

	return S_OK;
}

API_RESULT File::ReadStr(OUT char *pStr, OUT uint *bytes)
{
	uint fileSize = 0;

	FileSize(&fileSize);

	_file.read(pStr, fileSize);

	auto read_bytes = _file.gcount();

	pStr[read_bytes] = '\0';

	*bytes = (uint)read_bytes + 1;

	return S_OK;
}

API_RESULT File::IsEndOfFile(OUT int *eof)
{
	*eof = _file.eof();
	return S_OK;
}

API_RESULT File::Write(const uint8 *pMem, uint bytes)
{
	_file.write((const char *)pMem, bytes);
	return S_OK;
}

API_RESULT File::WriteStr(const char *pStr)
{
	_file.write(pStr, strlen(pStr));
	return S_OK;
}

API_RESULT File::FileSize(OUT uint *size)
{
	*size = (uint)file_size(_fsPath);
	return S_OK;
}

API_RESULT File::CloseAndFree()
{
	_file.close();
	delete this;
	return S_OK;
}
