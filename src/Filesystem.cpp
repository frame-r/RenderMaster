#include "Filesystem.h"
#include "Core.h"

using namespace std;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

FileSystem::FileSystem(const std::string& dataPath) :
	_dataPath(dataPath)
{
}

API FileSystem::OpenFile(OUT IFile **pFile, const char* pPath, FILE_OPEN_MODE mode)
{
	const bool read = (mode & FILE_OPEN_MODE::READ) == FILE_OPEN_MODE::READ;
	const bool write = (mode & FILE_OPEN_MODE::WRITE) == FILE_OPEN_MODE::WRITE;

	wstring wpPath = ConvertFromUtf8ToUtf16(pPath);

	fs::path wfsPath(wpPath);
	fs::path wfsFullPath(wpPath);

	if (wfsPath.is_relative())
	{
		wstring wdataPath = ConvertFromUtf8ToUtf16(_dataPath);
		wfsFullPath = wstring(wdataPath + L'\\' + wpPath);
	}

	if (!exists(wfsFullPath) && read)
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

	File *_pFile = new File(wfsFullPath.u8string().c_str(), cpp_mode, wfsFullPath);
	*pFile = _pFile;

	return S_OK;

}

API FileSystem::FileExist(const char* fullPath, OUT int *exist)
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

API FileSystem::GetName(OUT const char **pName)
{
	*pName = "FileSystem";
	return S_OK;
}

File::File(const char *pName, ios_base::openmode& cpp_mode, const fs::path& fullPath)
{
	_fs_full_path = fullPath;

	#if _WIN32 || _WIN64
		// native for windows is UTF-16 => convert pName UTF-16 to UTF-8
		wstring name = ConvertFromUtf8ToUtf16(pName);

		_file.open(name, cpp_mode);
	#else 
		// native for linux is UTF-8 => no need conversion
		_file.open(pName, cpp_mode);
	#endif
}

API File::Read(OUT uint8 *pMem, uint bytes)
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

API File::ReadStr(OUT char *pStr, OUT uint *bytes)
{
	uint file_size = 0;

	FileSize(&file_size);

	_file.read(pStr, file_size);

	auto read_bytes = _file.gcount();

	pStr[read_bytes] = '\0';

	*bytes = (uint)read_bytes + 1;

	return S_OK;
}

API File::IsEndOfFile(OUT int *eof)
{
	*eof = _file.eof();
	return S_OK;
}

API File::Write(const uint8 *pMem, uint bytes)
{
	_file.write((const char *)pMem, bytes);
	return S_OK;
}

API File::WriteStr(const char *pStr)
{
	_file.write(pStr, strlen(pStr));
	return S_OK;
}

API File::FileSize(OUT uint *size)
{
	*size = (uint)file_size(_fs_full_path);
	return S_OK;
}

API File::CloseAndFree()
{
	_file.close();
	delete this;
	return S_OK;
}
