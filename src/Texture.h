#pragma once
#include "Common.h"

class Texture : public ITexture
{
	ICoreTexture *_coreTexture = nullptr;

public:
	Texture(ICoreTexture *tex, int isShared, const string& fileIn) : _coreTexture(tex), _isShared(isShared), _file(fileIn) {}
	virtual ~Texture();

	API GetCoreTexture(ICoreTexture **texOut) override;
	API GetWidth(OUT uint *w) override;
	API GetHeight(OUT uint *h) override;
	API GetFormat(OUT TEXTURE_FORMAT *formatOut) override;

	BASE_RESOURCE_HEADER
};
