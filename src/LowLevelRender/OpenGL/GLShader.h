#pragma once
#include "Common.h"

class GLShader : public ICoreShader
{
	GLuint _programID = 0u;
	GLuint _vertID = 0u;
	GLuint _geomID = 0u;
	GLuint _fragID = 0u;

	// all buffers need to bind for work with shader
	// slot -> index of UBO in UBOpool array
	std::vector<size_t> _bufferIndicies;

	struct Parameter
	{
		int bufferIndex = -1; // index of UBO in UBOpool
		int parameterIndex = -1; // index in UBO::parameters
	};
	std::unordered_map<string, Parameter> _parameters; // all shader parameters	

	void setParameter(const char *name, const void *data);

public:

	GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID);
	virtual ~GLShader();

	GLuint programID() const { return _programID; }
	void bind();
	
	API SetFloatParameter(const char* name, float value) override;
	API SetVec4Parameter(const char* name, const vec4 *value) override;
	API SetMat4Parameter(const char* name, const mat4 *value) override;
	API SetUintParameter(const char* name, uint value) override;
	API FlushParameters() override;
};

