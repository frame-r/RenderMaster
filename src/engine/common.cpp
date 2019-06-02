#include "pch.h"
#include "common.h"
#include "mesh.h"
#include "texture.h"
#include "resource_manager.h"
#include "core.h"
#include "console.h"
#include <time.h>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

static std::set<int> instances_id;

string msaa_to_string(int samples)
{
	if (samples <= 1)
		return "no";
	return std::to_string(samples) + "x";
}

string ConvertFromUtf16ToUtf8(const std::wstring& wstr)
{
	if (wstr.empty())
		return string();

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
		std::vector<wchar_t> buffer(size);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], size);
		res.assign(buffer.begin(), buffer.end() - 1);
	}
	return res;
}

template<typename Char>
std::basic_string<Char> ToLowerCase(std::basic_string<Char> str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

std::string fileExtension(const std::string& path)
{
	return ToLowerCase(fs::path(path).extension().string().erase(0, 1));
}

int currentTime()
{
	return (int)time(NULL);
}

bool isIsFree(int id)
{
	return instances_id.find(id) != instances_id.end();
}

void addId(int id)
{
	instances_id.insert(id);
}

size_t blockSize(TEXTURE_FORMAT compressedFormat)
{
	assert(compressedFormat == TEXTURE_FORMAT::DXT1 || compressedFormat == TEXTURE_FORMAT::DXT3 || compressedFormat == TEXTURE_FORMAT::DXT5);
	if (compressedFormat == TEXTURE_FORMAT::DXT1)
		return 8u;
	return 16u;
}

size_t bytesPerPixel(TEXTURE_FORMAT format)
{
	switch (format)
	{
		case TEXTURE_FORMAT::R8:		return 1;
		case TEXTURE_FORMAT::RG8:		return 2;
		case TEXTURE_FORMAT::RGBA8:		return 4;
		case TEXTURE_FORMAT::R16F:		return 1;
		case TEXTURE_FORMAT::RG16F:		return 2;
		case TEXTURE_FORMAT::RGBA16F:	return 4;
		case TEXTURE_FORMAT::R32F:		return 4;
		case TEXTURE_FORMAT::RG32F:		return 8;
		case TEXTURE_FORMAT::RGBA32F:	return 16;
		case TEXTURE_FORMAT::R32UI:		return 4;
		case TEXTURE_FORMAT::D24S8:		return 4;
		default: assert(false); // no sense
	}
	return 1;
}

bool isColorFormat(TEXTURE_FORMAT format)
{
	if (format == TEXTURE_FORMAT::D24S8)
		return false;
	return true;
}

bool isCompressedFormat(TEXTURE_FORMAT format)
{
	if (format == TEXTURE_FORMAT::DXT1 || format == TEXTURE_FORMAT::DXT3 || format == TEXTURE_FORMAT::DXT5)
		return true;
	return false;
}

//void calculateTexture(size_t& numBytes, size_t& rowBytes, uint width, uint height, TEXTURE_FORMAT format)
//{
//	if (isCompressedFormat(format))
//	{
//		size_t bpb = blockSize(format);
//		size_t numBlocksWide = std::max<uint64_t>(1u, (uint64_t(width) + 3u) / 4u);
//		size_t numBlocksHigh = std::max<uint64_t>(1u, (uint64_t(height) + 3u) / 4u);
//		rowBytes = numBlocksWide * bpb;
//		numBytes = rowBytes * numBlocksHigh;
//	} else
//	{
//		size_t bpp = bytesPerPixel(format);
//		rowBytes = /*(*/uint64_t(width) * bpp/* + 7u) / 8u*/; // from https://github.com/Microsoft/DirectXTex/blob/master/DDSTextureLoader/DDSTextureLoader.cpp
//		numBytes = rowBytes * height;
//	}
//}

std::string bytesToMBytes(size_t bytes)
{
	std::string s(16, '\0');
	float mbytes = bytes / (1024.f * 1024.f);
	auto written = std::snprintf(&s[0], s.size(), "%.2f", mbytes);
	s.resize(written);	
	return s;
}

mat3 &orthonormalize(mat3 &ret, const mat3 &m)
{
	vec3 x = vec3(m.el_2D[0][0], m.el_2D[1][0], m.el_2D[2][0]);
	vec3 y = vec3(m.el_2D[0][1], m.el_2D[1][1], m.el_2D[2][1]);
	vec3 z = x.Cross(y);
	y = z.Cross(x);
	x.Normalize();
	y.Normalize();
	z.Normalize();
	ret.el_2D[0][0] = x.x;
	ret.el_2D[0][1] = y.x;
	ret.el_2D[0][2] = z.x;
	ret.el_2D[1][0] = x.y;
	ret.el_2D[1][1] = y.y;
	ret.el_2D[1][2] = z.y;
	ret.el_2D[2][0] = x.z;
	ret.el_2D[2][1] = y.z;
	ret.el_2D[2][2] = z.z;
	return ret;
}

inline bool IsNan(float f)
{
    const uint32 u = *(uint32*)&f;
    return (u&0x7F800000) == 0x7F800000 && (u&0x7FFFFF);
}

void decompositeTransform(const mat4 &transform, vec3 &t, quat &r, vec3 &s)
{
	mat3 rotate, rotation = mat3(transform);
	orthonormalize(rotate, rotation);
	t.x = transform.el_2D[0][3];
	t.y = transform.el_2D[1][3];
	t.z = transform.el_2D[2][3];
	r = quat(rotate);
	s.x = rotate.el_2D[0][0] * rotation.el_2D[0][0] + rotate.el_2D[1][0] * rotation.el_2D[1][0] + rotate.el_2D[2][0] * rotation.el_2D[2][0];
	s.y = rotate.el_2D[0][1] * rotation.el_2D[0][1] + rotate.el_2D[1][1] * rotation.el_2D[1][1] + rotate.el_2D[2][1] * rotation.el_2D[2][1];
	s.z = rotate.el_2D[0][2] * rotation.el_2D[0][2] + rotate.el_2D[1][2] * rotation.el_2D[1][2] + rotate.el_2D[2][2] * rotation.el_2D[2][2];

	assert(!IsNan(r.x));
	assert(!IsNan(r.y));
	assert(!IsNan(r.z));
	assert(!IsNan(r.w));
	assert(!IsNan(s.x));
	assert(!IsNan(s.y));
	assert(!IsNan(s.z));
	assert(!IsNan(t.x));
	assert(!IsNan(t.y));
	assert(!IsNan(t.z));
}

void compositeTransform(mat4& transform, const vec3& t, const quat& r, const vec3& s)
{
	mat4 R;
	mat4 T;
	mat4 S;

	R = r.ToMatrix();

	T.el_2D[0][3] = t.x;
	T.el_2D[1][3] = t.y;
	T.el_2D[2][3] = t.z;

	S.el_2D[0][0] = s.x;
	S.el_2D[1][1] = s.y;
	S.el_2D[2][2] = s.z;

	transform = T * R * S;
}

mat4 DLLEXPORT perspectiveRH_ZO(float fov, float aspect, float zNear, float zFar)
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

template <class T>
void ManagedPtr<T>::release()
{
	if (resource_)
	{
		resource_->decRef();
		if (resource_->getRefs() == 0)
		{
			resource_->free();
		}
		assert(resource_->getRefs()>=0);
		resource_ = nullptr;
	}
}

template<typename T>
Resource<T>::Resource(const string& path) :
	path_(path)
{
	Log("Resource created '%s'", path.c_str());
}

template <class T>
T* Resource<T>::get()
{
	if (pointer_)
	{
		frame_ = _core->frame();
		return pointer_.get();
	}
	if (loadingFailed)
		return nullptr;

	pointer_.reset(create());
	loadingFailed = !pointer_->Load();

	if (loadingFailed)
		pointer_ = nullptr;

	frame_ = _core->frame();
	return pointer_.get();
}

template class Resource<Mesh>;
template class Resource<Texture>;
template class DLLEXPORT ManagedPtr<Mesh>;
template class DLLEXPORT ManagedPtr<Texture>;
