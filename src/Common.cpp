#include "Common.h"
#include <experimental/filesystem>
#include "Core.h"

namespace fs = std::experimental::filesystem;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

using std::list;
using std::string;

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
	delete[] pText;
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

mat4 perspective(float fov, float aspect, float zNear, float zFar)
{
	assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

	float const tanHalfFovy = tan(fov / 2);

	mat4 Result;

	Result.el_2D[0][0] = 1.0f / (aspect * tanHalfFovy);
	Result.el_2D[1][1] = 1.0f / (tanHalfFovy);
	Result.el_2D[2][2] = -(zFar + zNear) / (zFar - zNear);
	Result.el_2D[3][2] = -1.0f;
	Result.el_2D[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
	
	return Result;
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

const char** make_char_pp(const list<string>& lines)
{
	char **ret = new char*[lines.size() + 1];

	memset(ret, 0, (lines.size() + 1) * sizeof(char*));

	int i = 0;
	for (auto it = lines.begin(); it != lines.end(); it++)
	{
		ret[i] = new char[it->size() + 1];
		strncpy(ret[i], it->c_str(), it->size());
		ret[i][it->size()] = '\0';
		i++;
	}

	return const_cast<const char**>(ret);
}

const char* make_char_p(const list<string>& lines)
{
	string out;
	for (const string& s : lines)
		out = out + s;
	char *ret = new char[out.size() + 1];
	strcpy(ret, out.c_str());
	ret[out.size()] = '\0';
	return ret;
}


std::list<std::string> get_file_content(const std::string& filename)
{
	IFile *pFile = nullptr;
	uint fileSize = 0;
	std::string textIn;
	int filseExist = 0;
	const char **textOut;
	int numLinesOut;

	char *pInstalledDir;
	_pCore->GetInstalledDir(&pInstalledDir);
	std::string installedDir = std::string(pInstalledDir);

	std::string shader_path = installedDir + '\\' + SHADER_DIR + '\\' + filename;

	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	fs->FileExist(const_cast<char*>(shader_path.c_str()), &filseExist);

	if (!filseExist)
	{
		LOG_WARNING_FORMATTED("ResourceManager::LoadShaderText(): File doesn't exist '%s'", shader_path.c_str());
		return std::list<std::string>();
	}

	fs->OpenFile(&pFile, shader_path.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

	pFile->FileSize(&fileSize);

	textIn.resize(fileSize);

	pFile->Read((uint8 *)textIn.c_str(), fileSize);
	pFile->CloseAndFree();

	split_by_eol(textOut, numLinesOut, textIn);

	std::list<string> l = make_lines_list(textOut);

	delete_char_pp(textOut);

	return l;

}


