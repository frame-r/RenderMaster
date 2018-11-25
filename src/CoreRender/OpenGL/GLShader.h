#pragma once
#include "Common.h"

class GLShader : public ICoreShader
{
	GLuint _programID = 0u;
	GLuint _vertID = 0u;
	GLuint _geomID = 0u;
	GLuint _fragID = 0u;

public:

	GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID);
	virtual ~GLShader();

	GLuint programID() const { return _programID; }
};

