#include "pch.h"
#include "texture.h"
#include "core.h"
#include "console.h"
#include "filesystem.h"
#include "icorerender.h"
#include "dds.h"

Texture::Texture(const std::string& path, TEXTURE_CREATE_FLAGS flags) : path_(path), flags_(flags)
{
}

Texture::Texture(std::unique_ptr<ICoreTexture> tex)
{
	coreTexture_ = std::move(tex);
}

Texture::~Texture()
{
	//Log("Texture destroyed: '%s'", path_.c_str());
}

bool Texture::Load()
{
	Log("Texture loading: '%s'", path_.c_str());

	if (!FS->FileExist(path_.c_str()))
	{
		LogCritical("Texture::Load(): file '%s' not found", path_.c_str());
		return false;
	}

	const string ext = fileExtension(path_.c_str());
	if (ext != "dds")
	{
		LogCritical("Texture::Load(): extension %s is not supported", ext.c_str());
		return false;
	}

	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	size_t size = f.FileSize();
	unique_ptr<uint8[]> fileData = std::make_unique<uint8[]>(size);

	f.Read((uint8 *)fileData.get(), size);

	ICoreTexture *coreTex = createDDS(fileData.get(), size, flags_);
	coreTexture_ = std::unique_ptr<ICoreTexture>(coreTex);

	if (!coreTex)
	{
		LogCritical("Texture::Load(): some error occured");
		return false;
	}
	
	return true;
}

auto DLLEXPORT Texture::GetCoreTexture() -> ICoreTexture *
{
	return coreTexture_.get();
}

auto DLLEXPORT Texture::GetVideoMemoryUsage() -> size_t
{
	if (!coreTexture_)
		return 0;

	return coreTexture_->GetVideoMemoryUsage();
}

auto DLLEXPORT Texture::GetWidth() -> int
{
	return coreTexture_->GetWidth();
}

auto DLLEXPORT Texture::GetHeight() -> int
{
	return coreTexture_->GetHeight();
}

auto DLLEXPORT Texture::GetMipmaps() -> int
{
	return coreTexture_->GetMipmaps();
}

auto DLLEXPORT Texture::ReadPixel2D(void *data, int x, int y) -> int
{
	return coreTexture_->ReadPixel2D(data, x, y);
}
