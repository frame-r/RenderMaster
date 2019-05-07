#pragma once
#include "icorerender.h"

class Texture final
{
	std::unique_ptr<ICoreTexture> _coreTexture;
	std::string path_;
	TEXTURE_CREATE_FLAGS flags_;

public:
	Texture(const std::string& path, TEXTURE_CREATE_FLAGS flags);
	Texture(std::unique_ptr<ICoreTexture> tex);
	~Texture();

	bool Load();

	auto DLLEXPORT GetCoreTexture() -> ICoreTexture*;
	auto DLLEXPORT GetVideoMemoryUsage() -> size_t;
	auto DLLEXPORT GetWidth() -> int;
	auto DLLEXPORT GetHeight() -> int;
	auto DLLEXPORT GetMipmaps() -> int;
	auto DLLEXPORT ReadPixel2D(void *data, int x, int y) -> int;
};
