#pragma once
#include "Common.h"

#include <GL\glew.h>


class GLShader : public ICoreShader
{
	GLuint _programID;
	GLuint _vertID;
	GLuint _geomID;
	GLuint _fragID;

public:

	GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID);
	~GLShader();

	API Free() override;
	API GetType(RES_TYPE& type) override;
};

