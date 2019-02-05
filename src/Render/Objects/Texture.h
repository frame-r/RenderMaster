#pragma once
#include "Common.h"

class Texture : public BaseResource<ITexture>
{
	unique_ptr<ICoreTexture> _coreTexture;

public:
	Texture(unique_ptr<ICoreTexture> tex);
	Texture(unique_ptr<ICoreTexture> tex, const string& path);

	API_RESULT GetCoreTexture(ICoreTexture **texOut) override;
	API_RESULT GetWidth(OUT uint *w) override;
	API_RESULT GetHeight(OUT uint *h) override;
	API_RESULT GetFormat(OUT TEXTURE_FORMAT *formatOut) override;
};
