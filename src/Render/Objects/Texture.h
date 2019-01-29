#pragma once
#include "Common.h"

class Texture : public ITexture
{
	unique_ptr<ICoreTexture> _coreTexture;

public:
	Texture(unique_ptr<ICoreTexture> tex);
	Texture(unique_ptr<ICoreTexture> tex, const string& path);

	API GetCoreTexture(ICoreTexture **texOut) override;
	API GetWidth(OUT uint *w) override;
	API GetHeight(OUT uint *h) override;
	API GetFormat(OUT TEXTURE_FORMAT *formatOut) override;

	BASE_RESOURCE_HEADER
};
