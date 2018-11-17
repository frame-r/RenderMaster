#pragma once
#include "Common.h"

class ShaderText : public IShaderText
{
	const char *vert = nullptr;
	const char *geom = nullptr;
	const char *frag = nullptr;

public:
	ShaderText(const char *vertIn, const char *geomIn, const char *fragIn, const string& fileIn) :
		vert(vertIn), geom(geomIn), frag(fragIn), _file(fileIn)	{}

	virtual ~ShaderText();

	API GetVert(OUT const char **text) override { *text = vert; return S_OK; }
	API GetGeom(OUT const char **text) override { *text = geom; return S_OK; }
	API GetFrag(OUT const char **text) override { *text = frag; return S_OK; }

	SHARED_ONLY_RESOURCE_HEADER
};
