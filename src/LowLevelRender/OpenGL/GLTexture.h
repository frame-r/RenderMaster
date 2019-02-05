#pragma once
#include "Common.h"

class GLTexture : public ICoreTexture
{
	GLuint _textureID = 0u;
	uint _width;
	uint _height;
	TEXTURE_FORMAT _format;

public:

	GLTexture(GLuint idIn, TEXTURE_FORMAT formatIn);
	virtual ~GLTexture();

	API_RESULT GetWidth(OUT uint *w) override { *w = _width; return S_OK; }
	API_RESULT GetHeight(OUT uint *h) override { *h = _height; return S_OK; }
	API_RESULT GetFormat(OUT TEXTURE_FORMAT *formatOut) override { *formatOut = _format; return S_OK; }

	UINT width() {return _width; }
	UINT height() {return _height; }
	GLuint textureID() const { return _textureID; }
};

