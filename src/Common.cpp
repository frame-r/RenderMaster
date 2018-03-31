#include "Common.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

bool is_relative(const char *pPath)
{
	std::wstring wpPath = ConvertFromUtf8ToUtf16(pPath);

	fs::path wfsPath(wpPath);

	return wfsPath.is_relative();
}

std::string make_absolute(const char* pRelDataPath, const char* pWorkingPath)
{
	std::wstring workingPath = ConvertFromUtf8ToUtf16(pWorkingPath);
	std::wstring relPath = ConvertFromUtf8ToUtf16(pRelDataPath);

	fs::path fbase(workingPath);
	fs::path frel(relPath);

	fs::path p = canonical(frel, fbase);

	return p.string();
}
