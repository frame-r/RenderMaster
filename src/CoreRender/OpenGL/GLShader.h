#pragma once
#include "Common.h"

class GLShader : public ICoreShader
{
	GLuint _programID {0};
	GLuint _vertID {0};
	GLuint _geomID {0};
	GLuint _fragID {0};

public:

	GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID);
	virtual ~GLShader();

	void Free();

	GLuint programID() const { return _programID; }
};

