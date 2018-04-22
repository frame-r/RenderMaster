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


void split_by_eol(char **&text, int &num_lines, const std::string& str)
{
	char **ret;
	char *p_begin_line = const_cast<char*>(&str[0]);
	int characters_num = 0;
	int line = 0;
	int number_of_lines = 1;

	for (uint i = 0; i < str.size(); i++)
	{
		if (str[i] == '\n') number_of_lines++;
	}

	ret = new char*[number_of_lines + 1];
	memset(ret, 0, (number_of_lines + 1) * sizeof(char*));

	for (uint i = 0; i < str.size() + 1; i++)
	{
		if (str[i] == '\n')
		{
			ret[line] = new char[characters_num + 2];
			strncpy(ret[line], p_begin_line, characters_num + 1);
			ret[line][characters_num + 1] = '\0';
			characters_num = 0;
			line++;
			p_begin_line = const_cast<char*>(&str[i + 1]);
		}
		else
			characters_num++;
	}

	if (str.back() == '\n') // last empty line
	{
		ret[line] = new char[1];
		ret[line][0] = '\0';
	}

	text = ret;
	num_lines = number_of_lines;
}

void delete_ptr_ptr_char(char **pText)
{
	if (!pText) return;
	char **c = pText;
	while(*c)
	{
		delete *c;
		*c = nullptr;
		c++;
	}
	delete pText;
}

