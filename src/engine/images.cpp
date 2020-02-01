#include "pch.h"
#include "images.h"
#include "core.h"
#include "texture.h"
#include "console.h"
#include "render.h"
#include "icorerender.h"
#include "resource_manager.h"
#include "filesystem.h"
#include "jpeglib.h"
#include "png.h"
#include "pngstruct.h"

struct MipmapGenerationResult
{
	std::unique_ptr<uint8[]> rgbaBuffer;
	uint mipmaps;
	int width, height;
	size_t bufferInBytes;
};

#pragma pack(push,1)

const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

struct DDS_PIXELFORMAT
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME 0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

enum DDS_MISC_FLAGS2
{
	DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT     dxgiFormat;
	uint32_t        resourceDimension;
	uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
	uint32_t        arraySize;
	uint32_t        miscFlags2;
};

TEXTURE_FORMAT D3DToEng(DXGI_FORMAT format);

#pragma pack(pop)

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )


ICoreTexture *createFromDDS(unique_ptr<uint8_t[]> dataPtr, size_t size, TEXTURE_CREATE_FLAGS flags)
{
	// Check magic
	uint32_t dwMagicNumber = *reinterpret_cast<const uint32_t*>(dataPtr.get());
	if (dwMagicNumber != DDS_MAGIC)
	{
		LogCritical("loadDDS(): Wrong magic");
		return nullptr;
	}

	const DDS_HEADER* header = reinterpret_cast<const DDS_HEADER*>(dataPtr.get() + sizeof(uint32_t));

	// Check header sizes
	if (header->size != sizeof(DDS_HEADER) || header->ddspf.size != sizeof(DDS_PIXELFORMAT))
	{
		LogCritical("loadDDS(): Wrong header sizes");
		return nullptr;
	}

	const bool mipmapsPresented = header->mipMapCount > 1;
	const uint width = header->width;
	const uint height = header->height;

	// Check for DX10 extension
	bool bDXT10Header = (header->ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC);

	ptrdiff_t headerOffset = sizeof(uint32_t) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);

	uint8_t *imageData = dataPtr.get() + headerOffset;
	size_t sizeInBytes = size - headerOffset;

	TEXTURE_TYPE type = TEXTURE_TYPE::TYPE_2D;
	TEXTURE_FORMAT format = TEXTURE_FORMAT::UNKNOWN;

	unique_ptr<uint8_t[]> newData;

	if (bDXT10Header)
	{
		const DDS_HEADER_DXT10* d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));

		if (d3d10ext->arraySize == 0)
			abort();

		switch (d3d10ext->dxgiFormat)
		{
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
		case DXGI_FORMAT_A8P8:
			abort();

		default: break;
		}

		format = D3DToEng(d3d10ext->dxgiFormat);

		switch (d3d10ext->resourceDimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			abort(); // not impl
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if (d3d10ext->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE)
				type = TEXTURE_TYPE::TYPE_CUBE;
			break;

		default:
			abort(); // not impl
		}
	}
	else
	{
		const DDS_PIXELFORMAT ddpf = header->ddspf;

		if (ddpf.flags & DDS_RGB)
		{
			// Note that sRGB formats are written using the "DX10" extended header
			switch (ddpf.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
					format = TEXTURE_FORMAT::RGBA8;
				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
					format = TEXTURE_FORMAT::BGRA8;
				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000)) // DXGI_FORMAT_B8G8R8X8_UNORM

				// Convert BGR<unused> -> RGBA
				{
					newData = unique_ptr<uint8_t[]>(new uint8[sizeInBytes]);
					memset(newData.get(), 255, sizeInBytes);

					uint8* ptrSrc = imageData;
					uint8* ptrDst = newData.get();

					size_t texels = (size_t)header->width * header->height;

					for (uint32_t m = 0; m < header->mipMapCount; ++m)
					{
						for (size_t i = 0; i < texels; ++i)
						{
							// BGR -> RGB
							memcpy((ptrDst + 0), (ptrSrc + 2), 1);
							memcpy((ptrDst + 1), (ptrSrc + 1), 1);
							memcpy((ptrDst + 2), (ptrSrc + 0), 1);

							ptrDst += 4;
							ptrSrc += 4;
						}
						texels /= 4;
					}

					format = TEXTURE_FORMAT::RGBA8;
				}
				

				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assume
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				//if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				//{
				//	//return DXGI_FORMAT_R10G10B10A2_UNORM;
				//	break;
				//}

				//// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

				//if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				//{
				//	//return DXGI_FORMAT_R16G16_UNORM;
				//	break;
				//}

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
				{
					// Only 32-bit color channel format in D3D9 was R32F
					format = TEXTURE_FORMAT::R32F;
				}
				break;

			case 24:
				// No 24bpp DXGI formats aka D3DFMT_R8G8B8
				// Convert BGR -> RGBA
				{
					newData = convertRGBtoRGBA(imageData, width, height, true, true);

					format = TEXTURE_FORMAT::RGBA8;
				}
				break;

			case 16:
				//if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				//{
				//	//return DXGI_FORMAT_B5G5R5A1_UNORM;
				//	break;
				//}
				//if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
				//{
				//	//return DXGI_FORMAT_B5G6R5_UNORM;
				//	break;
				//}

				//// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

				//if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				//{
				//	//return DXGI_FORMAT_B4G4R4A4_UNORM;
				//	break;
				//}

				// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

				// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE)
		{
			if (8 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
				{
					format = TEXTURE_FORMAT::R8; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				{
					format = TEXTURE_FORMAT::RG8; // Some DDS writers assume the bitcount should be 8 instead of 16
				}
			}

			if (16 == ddpf.RGBBitCount)
			{
				//if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
				//{
				//	//return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				//}
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				{
					format = TEXTURE_FORMAT::RG8; // D3DX10/11 writes this out as DX10 extension
				}
			}
		}
		else if (ddpf.flags & DDS_ALPHA)
		{
			if (8 == ddpf.RGBBitCount)
			{
				format = TEXTURE_FORMAT::R8;
			}
		}
		else if (ddpf.flags & DDS_BUMPDUDV)
		{
			//if (16 == ddpf.RGBBitCount)
			//{
			//	if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
			//	{
			//		//return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
			//	}
			//}

			//if (32 == ddpf.RGBBitCount)
			//{
			//	if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
			//	{
			//		//return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
			//	}
			//	if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
			//	{
			//		//return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
			//	}

			//	// No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
			//}
		}
		else if (ddpf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
				format = TEXTURE_FORMAT::DXT1;
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
				format = TEXTURE_FORMAT::DXT3;
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
				format = TEXTURE_FORMAT::DXT5;

			// While pre-multiplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			//if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC2_UNORM;
			//}
			//if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC3_UNORM;
			//}

			//if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC4_UNORM;
			//}
			//if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC4_UNORM;
			//}
			//if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC4_SNORM;
			//}

			//if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC5_UNORM;
			//}
			//if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC5_UNORM;
			//}
			//if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_BC5_SNORM;
			//}

			// BC6H and BC7 are written using the "DX10" extended header

			//if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_R8G8_B8G8_UNORM;
			//}
			//if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_G8R8_G8B8_UNORM;
			//}

			//if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
			//{
			//	//return DXGI_FORMAT_YUY2;
			//}

			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				//return DXGI_FORMAT_R16G16B16A16_UNORM;
				break;

			case 110: // D3DFMT_Q16W16V16U16
				format = TEXTURE_FORMAT::RGBA16F; break;

			case 111: // D3DFMT_R16F
				format = TEXTURE_FORMAT::R16F; break;

			case 112: // D3DFMT_G16R16F
				format = TEXTURE_FORMAT::RG16F; break;

			case 113: // D3DFMT_A16B16G16R16F
				format = TEXTURE_FORMAT::RGBA16F; break;

			case 114: // D3DFMT_R32F
				format = TEXTURE_FORMAT::R32F; break;

			case 115: // D3DFMT_G32R32F
				format = TEXTURE_FORMAT::RG32F; break;

			case 116: // D3DFMT_A32B32G32R32F
				//return TEXTURE_FORMAT::RGBA32F;
				break;
			}
		}

		if (format == TEXTURE_FORMAT::UNKNOWN)
			abort();
	
		if (header->flags & DDS_HEADER_FLAGS_VOLUME)
		{
			abort();// Volume not implemented
		}
		else
		{
			if (header->caps2 & DDS_CUBEMAP)
			{
				// We require all six faces to be defined
				if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
					abort();
				
				type = TEXTURE_TYPE::TYPE_CUBE;
			}
		}
	}

	if (format == TEXTURE_FORMAT::UNKNOWN)
	{
		LogCritical("ResourceManager::loadDDS(): format not supported");
		return nullptr;
	}

	ICoreTexture *tex = nullptr;

	if (newData)
	{
		imageData = newData.get();

		// free original memory
		dataPtr = nullptr;
	}

	ICoreTexture *ret = CORE_RENDER->CreateTexture(imageData, width, height, type, format, flags, mipmapsPresented);

	if (!ret)
	{
		LogCritical("ResourceManager::loadDDS(): failed to create texture");
		return nullptr;
	}

	return ret;
}

static MipmapGenerationResult generateMipmapsGPU(uint width, uint height, const uint8_t* rbgaBufferIn)
{
	const size_t baseSize = (size_t)width * height * 4;

	uint w = width;
	uint h = height;
	size_t offset = baseSize;
	uint32_t mipmaps = 1;

	unique_ptr<uint8_t[]> buffer;
	size_t bufferInBytes = baseSize;

	while (w > 1 || h > 1)
	{
		w = max(1u, w / 2);
		h = max(1u, h / 2);
		++mipmaps;
		bufferInBytes += (size_t)w * h * 4;
	}
	bufferInBytes += 1;

	buffer = unique_ptr<uint8_t[]>(new uint8_t[bufferInBytes]);
	memcpy(buffer.get(), rbgaBufferIn, baseSize);

	CORE_RENDER->PushStates();

	w = width;
	h = height;

	const uint8_t* bufferIn = buffer.get();

	for (uint mip = 1; mip < mipmaps; ++mip)
	{
		ICoreTexture* coreTexIn = CORE_RENDER->CreateTexture(bufferIn, w, h, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS::NONE, false);
		unique_ptr<Texture> texIn(new Texture(unique_ptr<ICoreTexture>(coreTexIn)));
		Texture* texsIn[] = { texIn.get() };
		CORE_RENDER->BindTextures(1, texsIn, BIND_TETURE_FLAGS::COMPUTE);

		uint newWidth = max(1u, w / 2);
		uint newHeight = max(1u, h / 2);

		vector<string> defines;

		if (newWidth * 2 == w)
			defines.push_back("X_EVEN");
		if (newHeight * 2 == h)
			defines.push_back("Y_EVEN");

		Shader* shader = RENDER->GetComputeShader("mipmap.hlsl", &defines);
		if (shader)
		{
			CORE_RENDER->SetShader(shader);
			shader->SetUintParameter("tex_in_width", w);
			shader->SetUintParameter("tex_in_height", h);
			shader->FlushParameters();

			w = newWidth;
			h = newHeight;
			const size_t mipSize = (size_t)w * h * 4;

			ICoreTexture* coreTexOut = CORE_RENDER->CreateTexture(nullptr, w, h, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS::USAGE_UNORDRED_ACCESS, false);
			unique_ptr<Texture> texOut(new Texture(unique_ptr<ICoreTexture>(coreTexOut)));
			Texture* uavs[] = { texOut.get() };
			CORE_RENDER->CSBindUnorderedAccessTextures(1, uavs);

			constexpr uint warpSize = 16;
			int numGroupsX = (w + (warpSize - 1)) / warpSize;
			int numGroupsY = (h + (warpSize - 1)) / warpSize;

			CORE_RENDER->Dispatch(numGroupsX, numGroupsY, 1);

			texOut->GetData(buffer.get() + offset, mipSize);

			bufferIn = buffer.get() + offset;
			offset += mipSize;
		}
	}

	Texture* texsIn[1] = {};
	CORE_RENDER->BindTextures(1, texsIn);

	Texture* uavs[1] = {};
	CORE_RENDER->CSBindUnorderedAccessTextures(1, uavs);

	CORE_RENDER->PopStates();

	MipmapGenerationResult ret;
	ret.mipmaps = mipmaps;
	ret.rgbaBuffer = std::move(buffer);
	ret.width = width;
	ret.height = height;
	ret.bufferInBytes = bufferInBytes;

	return ret;
}

static void saveDDS(const MipmapGenerationResult& mipmaped, const char* fullPathSrc)
{
	size_t headerInBytes = sizeof(uint32_t) + sizeof(DDS_HEADER);
	size_t ddsFileInBytes = headerInBytes + mipmaped.bufferInBytes;

	unique_ptr<uint8_t[]> ddsImage(new uint8_t[ddsFileInBytes]);

	memcpy(ddsImage.get(), &DDS_MAGIC, 4);
	memset(ddsImage.get() + 4, 0, sizeof(DDS_HEADER));

	DDS_HEADER* header = reinterpret_cast<DDS_HEADER*>(ddsImage.get() + 4);
	header->size = sizeof(DDS_HEADER);
	header->ddspf.size = sizeof(DDS_PIXELFORMAT);
	header->mipMapCount = mipmaped.mipmaps;
	header->width = mipmaped.width;
	header->height = mipmaped.height;
	header->depth = 1;

	DDS_PIXELFORMAT* ddpf = &header->ddspf;
	ddpf->flags = DDS_RGB;
	ddpf->RGBBitCount = 32;
	ddpf->RBitMask = 0x000000FF;
	ddpf->GBitMask = 0x0000FF00;
	ddpf->BBitMask = 0x00FF0000;
	ddpf->ABitMask = 0xFF000000;

	uint8* imageData = ddsImage.get() + headerInBytes;
	memcpy(imageData, mipmaped.rgbaBuffer.get(), mipmaped.bufferInBytes);

	auto filename = FS->GetFileName(fullPathSrc, false);
	string p = RES_MAN->GetImportMeshDir() + '\\' + filename + ".dds";

	File f = FS->OpenFile(p.c_str(), FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);

	f.Write(ddsImage.get(), ddsFileInBytes);

	Log("Saved to '%s'", p.c_str());
}

void importJPEG(const char* fullPath)
{
	File file = FS->OpenFile(fullPath, FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	size_t fileSize = file.FileSize();

	if (fileSize <= 0)
	{
		LogCritical("importJPEG(): file is empty");
		return;
	}

	unique_ptr<uint8_t[]> input(new uint8_t[fileSize]);
	file.Read(input.get(), fileSize);

	jpeg_decompress_struct cinfo{};
	jpeg_error_mgr pub{};

	cinfo.err = jpeg_std_error(&pub);

	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, input.get(), (unsigned long)fileSize);

	jpeg_read_header(&cinfo, TRUE);

	cinfo.out_color_space = JCS_RGB;
	cinfo.out_color_components = 3;
	cinfo.do_fancy_upsampling = FALSE;

	jpeg_start_decompress(&cinfo);

	auto rowspan = cinfo.image_width * cinfo.out_color_components;
	const uint width = cinfo.image_width;
	const uint height = cinfo.image_height;

	unique_ptr<uint8_t[]> output(new uint8[rowspan * (size_t)height]);
	uint8** row_ptr = new uint8 * [height];

	for (uint i = 0; i < height; ++i)
		row_ptr[i] = &output[i * rowspan];

	uint rowsRead = 0;

	while (cinfo.output_scanline < cinfo.output_height)
		rowsRead += jpeg_read_scanlines(&cinfo, &row_ptr[rowsRead], cinfo.output_height - rowsRead);

	delete[] row_ptr;

	jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	// RGB -> RGBA
	unique_ptr<const uint8_t[]> outputRGBA = convertRGBtoRGBA(output.get(), width, height, false, false);
	output = nullptr;

	MipmapGenerationResult mipmaps = generateMipmapsGPU(width, height, outputRGBA.get());
}

struct ImageSource
{
	unsigned char* data;
	size_t size;
	size_t offset;
};

static void PNGAPI pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ImageSource* isource = (ImageSource*)png_get_io_ptr(png_ptr);

	if (isource->offset + length <= isource->size)
	{
		memcpy(data, isource->data + isource->offset, length);
		isource->offset += length;
	}
	else
		LogCritical("pngReadCallback failed");
}

static void pngError(png_structp ptr, png_const_charp msg)
{
	LogCritical("PNG fatal error: %s", msg);
}

void importPNG(const char* path)
{
	File file = FS->OpenFile(path, FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	size_t fileSize = file.FileSize();

	if (fileSize <= 0)
	{
		LogCritical("importJPEG(): file is empty");
		return;
	}

	unique_ptr<uint8_t[]> input(new uint8_t[fileSize]);
	file.Read(input.get(), fileSize);

	if (png_sig_cmp(input.get(), 0, 8) != 0)
	{
		LogCritical("Wrong PNG file.");
		return;
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, (png_error_ptr)pngError, NULL);
	
	if (!png_ptr)
	{
		LogCritical("Internal PNG png_create_read_struct function failure.");
		return;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		LogCritical("Internal PNG png_create_info_struct function failure.");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return;
	}

	//if (setjmp(png_jmpbuf(png_ptr)))
	//{
	//	LogCritical("Internal PNG jump failure.");
	//	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	//	return;
	//}

	ImageSource imgsource;
	imgsource.data = input.get();
	imgsource.size = fileSize;
	imgsource.offset = 0;
	png_set_read_fn(png_ptr, &imgsource, pngReadCallback);

	//png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_uint_32 width, height;
	int bit_depth, color_type;

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (bit_depth < 8)
	{
		if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_expand_gray_1_2_4_to_8(png_ptr);
		else
			png_set_packing(png_ptr);
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	uint8_t** row_pointers = new png_bytep[height];

	if (!row_pointers)
	{
		LogCritical("Failed to allocate PNG row pointers.");
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return;
	}

	int pitch = (color_type == PNG_COLOR_TYPE_RGB_ALPHA ? 4 : 3) * width;
	uint8_t* data = new uint8_t[(size_t)height * pitch];
	uint8_t* offset = data;

	for (uint i = 0; i < height; ++i)
	{
		row_pointers[i] = offset;
		offset += pitch;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		LogCritical("Internal PNG jump failure.");
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		delete[] row_pointers;
		delete[] data;
		return;
	}

	png_read_image(png_ptr, row_pointers);

	png_read_end(png_ptr, NULL);
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	delete[] row_pointers;

	MipmapGenerationResult mipmapedBuffer;

	// RGB -> RGBA
	if (color_type == PNG_COLOR_TYPE_RGB)
	{
		unique_ptr<uint8_t[]> outputRGBA = convertRGBtoRGBA(data, width, height, false, false);
		mipmapedBuffer = generateMipmapsGPU(width, height, outputRGBA.get());
	} else
		mipmapedBuffer = generateMipmapsGPU(width, height, data);

	delete[] data;

	saveDDS(mipmapedBuffer, path);
}

