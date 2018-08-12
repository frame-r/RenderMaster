// Preprocessor.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_WARNINGS
#include "Preprocessor.h"
#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <list>
namespace fs = std::experimental::filesystem;
using namespace std;

#define SHADER_DIR "..\\src\\shaders\\"

void split_by_eol(const char **&textOut, int &numLinesOut, const std::string& textIn);
void delete_char_pp(const char **pText);
list<string> make_lines_list(const char **text);
list<string> get_file_content(const string& filename);


int main()
{		
	auto lines_lang = get_file_content("language_gl.h");
	auto lines = get_file_content("mesh_vertex.shader");
	lines.insert(lines.begin(), lines_lang.begin(), lines_lang.end());

	Preprocessor proc;
	proc.SetDefine("ENG_SHADER_VERTEX");
	proc.SetDefine("ENG_INPUT_COLOR");
	proc.Run(lines);

	fs::path wfsPathOut(SHADER_DIR"out.shader");
	std::fstream file_out(wfsPathOut.c_str(), ofstream::out);
	for (auto& ll : lines)
	{
		file_out.write(ll.c_str(), ll.size());
	}
	file_out.close();	

    return 0;
}

list<string> get_file_content(const string& filename)
{
#ifdef PREPOCESSOR_STANDALONE
	string textIn;
	fs::path wfsPath(SHADER_DIR + filename);

	if (!exists(wfsPath))
	{
		cout << "file isn't exist";
		return list<string>();
	}

	std::fstream file(wfsPath.c_str(), ofstream::in || ofstream::binary);

	int size = fs::file_size(wfsPath);
	textIn.resize(size + 1);

	file.read(const_cast<char*>(textIn.c_str()), size);

	auto read_bytes = file.gcount();
	textIn[read_bytes] = '\0';

	file.close();

	const char **textOut;
	int numLinesOut;

	split_by_eol(textOut, numLinesOut, textIn);
	auto lines = make_lines_list(textOut);
	delete_char_pp(textOut);

	return lines;
#else
	return list<string>();
#endif
}

void split_by_eol(const char **&textOut, int &numLinesOut, const std::string& textIn)
{
	char **ret;
	char *p_begin_line = const_cast<char*>(&textIn[0]);
	int characters_num = 0;
	int line = 0;
	int number_of_lines = 1;

	for (int i = 0; i < textIn.size(); i++)
	{
		if (textIn[i] == '\n') number_of_lines++;
	}

	ret = new char*[number_of_lines + 1];
	memset(ret, 0, (number_of_lines + 1) * sizeof(char*));

	for (int i = 0; i < textIn.size() + 1; i++)
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
	while (*c)
	{
		delete *c;
		*c = nullptr;
		c++;
	}
	delete pText;
}

list<string> make_lines_list(const char **text)
{
	list<string> ret;

	char **c = const_cast<char**>(text);

	while (*c)
	{
		ret.push_back(string(*c));
		c++;
	}

	return ret;
}
