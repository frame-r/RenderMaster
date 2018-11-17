#pragma once
#include "Common.h"

class Texture : public ITexture
{
	ICoreTexture *_coreTexture = nullptr;

public:
	Texture(ICoreTexture *tex, int isShared, const string& fileIn) : _coreTexture(tex), _isShared(isShared), _file(fileIn) {}
	virtual ~Texture();

	API GetCoreTexture(ICoreTexture **texOut) override;

	BASE_RESOURCE_HEADER
};
