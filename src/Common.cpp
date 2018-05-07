#include "Common.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

bool is_relative(const char *pPath)
{
	std::wstring wpPath = ConvertFromUtf8ToUtf16(pPath);

	fs::path wfsPath(wpPath);

	return wfsPath.is_relative();
}

std::string make_absolute(const char* pRelDataPath, const char* pBasePath)
{
	std::wstring workingPath = ConvertFromUtf8ToUtf16(pBasePath);
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

void look_at(Matrix4x4& Result, const Vector3 &eye, const Vector3 &center)
{
	Result = Matrix4x4(1.0f);
	Vector3 Z = (center - eye).Normalize();
	Vector3 X = Vector3(0.0f, 1.0f, 0.0f).Cross(Z).Normalize();
	Vector3 Y(Z.Cross(X));
	Y.Normalize();
	Result.el_2D[0][0] = X.x;
	Result.el_2D[0][1] = X.y;
	Result.el_2D[0][2] = X.z;
	Result.el_2D[1][0] = Y.x;
	Result.el_2D[1][1] = Y.y;
	Result.el_2D[1][2] = Y.z;
	Result.el_2D[2][0] = Z.x;
	Result.el_2D[2][1] = Z.y;
	Result.el_2D[2][2] = Z.z;
	Result.el_2D[0][3] = -X.Dot(eye);
	Result.el_2D[1][3] = -Y.Dot(eye);
	Result.el_2D[2][3] = -Z.Dot(eye);
}

