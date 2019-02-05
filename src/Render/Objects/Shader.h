#pragma once
#include "Common.h"

class Shader final : public BaseResource<IShader>
{
	unique_ptr<ICoreShader> _coreShader;
	unique_ptr<const char> _vertFullText;
	unique_ptr<const char> _geomFullText;
	unique_ptr<const char> _fragFullText;

public:
	Shader(unique_ptr<ICoreShader> s, unique_ptr<const char> vertIn, unique_ptr<const char> geomIn, unique_ptr<const char> fragIn);

	API GetCoreShader(ICoreShader **shaderOut) override;
	API GetVert(OUT const char **textOut) override;
	API GetGeom(OUT const char **textOut) override;
	API GetFrag(OUT const char **textOut) override;
	API SetFloatParameter(const char* name, float value) override;
	API SetVec4Parameter(const char* name, const vec4 *value) override;
	API SetMat4Parameter(const char* name, const mat4 *value) override;
	API SetUintParameter(const char* name, uint value) override;
	API FlushParameters() override;
};
