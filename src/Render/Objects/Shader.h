#pragma once
#include "Common.h"

class Shader : public IShader
{
	ICoreShader *_coreShader = nullptr;

	const char *vertText = nullptr;
	const char *geomText = nullptr;
	const char *fragText = nullptr;

public:
	Shader(ICoreShader *s, const char *vertIn, const char *geomIn, const char *fragIn) :
		_coreShader(s), vertText(vertIn), geomText(geomIn), fragText(fragIn) {}
	virtual ~Shader();

	API GetCoreShader(ICoreShader **shaderOut) override;
	API GetVert(OUT const char **textOut) override;
	API GetGeom(OUT const char **textOut) override;
	API GetFrag(OUT const char **textOut) override;
	API SetFloatParameter(const char* name, float value) override;
	API SetVec4Parameter(const char* name, const vec4 *value) override;
	API SetMat4Parameter(const char* name, const mat4 *value) override;
	API SetUintParameter(const char* name, uint value) override;
	API FlushParameters() override;


	RUNTIME_ONLY_RESOURCE_HEADER
};
