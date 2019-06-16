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

/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение
	одинарных, двойных, тройных и всех нечетных ошибок
*/
const static unsigned short Crc16Table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
	0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
	0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
	0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
	0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
	0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
	0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
	0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
	0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
	0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
	0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
	0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
	0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
	0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
	0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
	0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
	0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
	0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
	0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
	0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
	0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
	0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
	0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

unsigned short Crc16(unsigned char* pcBlock, unsigned short len)
{
	unsigned short crc = 0xFFFF;

	while (len--)
		crc = (crc << 8) ^ Crc16Table[(crc >> 8) ^ *pcBlock++];

	return crc;
}
