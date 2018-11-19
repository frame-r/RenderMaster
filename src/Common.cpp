#include "pch.h"
#include "Common.h"
#include "Core.h"
#include "GameObject.h"
#include "Model.h"
#include "Camera.h"
#include "SceneManager.h"

namespace fs = std::experimental::filesystem;

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

using std::list;

int is_relative(const char *pPath)
{
	fs::path fsPath = fs::u8path(pPath);
	return fsPath.is_relative();
}

string make_absolute(const char *pRelDataPath, const char *pBasePath)
{
	fs::path fsBase = fs::u8path(pBasePath);
	fs::path fsRel = fs::u8path(pRelDataPath);

	fs::path p = canonical(fsRel, fsBase);

	return p.u8string();
}

void split_by_eol(const char **&textOut, int &numLinesOut, const string& textIn)
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

template<typename Out>
void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
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

mat4 perspectiveRH_ZO(float fov, float aspect, float zNear, float zFar)
{
	assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

	float const tanHalfFovy = tan(fov / 2);

	mat4 Result;

	Result.el_2D[3][3] = 0.0f;
	Result.el_2D[0][0] = 1.0f / (aspect * tanHalfFovy);
	Result.el_2D[1][1] = 1.0f / (tanHalfFovy);
	Result.el_2D[2][2] = -zFar / (zFar - zNear);
	Result.el_2D[3][2] = -1.0f;
	Result.el_2D[2][3] = -(zFar * zNear) / (zFar - zNear);
	
	return Result;
}

int initialized = 0;
int seed = 0;
std::set<int> instances_id;

unsigned int random8()
{
	if (initialized == 0)
	{
		seed = (int)time(NULL);
		initialized = 1;
	}
	seed = seed * 1664525 + 1013904223;
	return (int)((seed >> 20) & 0xff);
}

int getRandomInt()
{
	int newid;
	do
	{
		newid = random8();
		newid |= random8() << 8;
		newid |= random8() << 16;
		newid |= (random8() & 0x7f) << 24;
	} while (instances_id.find(newid) != instances_id.end() && newid == 0);
	return newid;
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


string ConvertFromUtf16ToUtf8(const std::wstring& wstr)
{
	if (wstr.empty()) return string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

std::wstring ConvertFromUtf8ToUtf16(const string& str)
{
	std::wstring res;
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
	if (size > 0)
	{
		vector<wchar_t> buffer(size);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], size);
		res.assign(buffer.begin(), buffer.end() - 1);
	}
	return res;
}

std::list<string> get_file_content(const string& filename)
{
	IFile *pFile = nullptr;
	uint fileSize = 0;
	string textIn;
	int filseExist = 0;
	const char **textOut;
	int numLinesOut;

	const char *pInstalledDir;
	_pCore->GetInstalledDir(&pInstalledDir);
	string installedDir = string(pInstalledDir);

	string shader_path = installedDir + '\\' + SHADER_DIR + '\\' + filename;

	IFileSystem *fs;
	_pCore->GetSubSystem((ISubSystem**)&fs, SUBSYSTEM_TYPE::FILESYSTEM);
	fs->FileExist(const_cast<char*>(shader_path.c_str()), &filseExist);

	if (!filseExist)
	{
		LOG_WARNING_FORMATTED("ResourceManager::LoadShaderText(): File doesn't exist '%s'", shader_path.c_str());
		return std::list<string>();
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

// core render

int get_msaa_samples(INIT_FLAGS flags)
{
	if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_2X) return 2;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_4X) return 4;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_8X) return 8;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_16X) return 16;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_32X) return 32;
	return 0;
}

string msaa_to_string(int samples)
{
	if (samples <= 1)
		return "no";
	return std::to_string(samples) + "x";
}


