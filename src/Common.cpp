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


void split_by_eol(const char **&textOut, int &numLinesOut, const std::string& textIn)
{
	char **ret;
	char *p_begin_line = const_cast<char*>(&textIn[0]);
	int characters_num = 0;
	int line = 0;
	int number_of_lines = 1;

	for (uint i = 0; i < textIn.size(); i++)
	{
		if (textIn[i] == '\n') number_of_lines++;
	}

	ret = new char*[number_of_lines + 1];
	memset(ret, 0, (number_of_lines + 1) * sizeof(char*));

	for (uint i = 0; i < textIn.size() + 1; i++)
	{
		if (textIn[i] == '\n')
		{
			ret[line] = new char[characters_num + 2];
			strncpy(ret[line], p_begin_line, characters_num + 1);
			ret[line][characters_num + 1] = '\0';
			characters_num = 0;
			line++;
			p_begin_line = const_cast<char*>(&textIn[i + 1]);
		}
		else
			characters_num++;
	}

	if (textIn.back() == '\n') // last empty line
	{
		ret[line] = new char[1];
		ret[line][0] = '\0';
	}

	textOut = const_cast<const char**>(ret);
	numLinesOut = number_of_lines;
}

void delete_char_pp(const char **pText)
{
	if (!pText) return;
	char **c = const_cast<char**>(pText);
	while(*c)
	{
		delete *c;
		*c = nullptr;
		c++;
	}
	delete pText;
}

