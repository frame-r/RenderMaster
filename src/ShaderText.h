#pragma once
#include "Common.h"

class ShaderText : public IShaderText
{
	const char *text = nullptr;

public:
	ShaderText(const char *textIn, const string& fileIn) :
		text(textIn), _file(fileIn)	{}

	virtual ~ShaderText();

	API GetText(OUT const char **textOut) override { *textOut = text; return S_OK; }
	API SetText(const char *textIn) override;

	SHARED_ONLY_RESOURCE_HEADER
};
