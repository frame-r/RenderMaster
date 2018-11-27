#pragma once
#include "Common.h"

class Texture : public ITexture
{
	ICoreTexture *_coreTexture = nullptr;

public:
	Texture(ICoreTexture *tex) : _coreTexture(tex) {}
	Texture(ICoreTexture *tex, const string& filePath) : _coreTexture(tex), _file(filePath) {}
	virtual ~Texture();

	API GetCoreTexture(ICoreTexture **texOut) override;
	API GetWidth(OUT uint *w) override;
	API GetHeight(OUT uint *h) override;
	API GetFormat(OUT TEXTURE_FORMAT *formatOut) override;

	BASE_RESOURCE_HEADER
};
