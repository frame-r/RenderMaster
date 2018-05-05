#pragma once
#include "Common.h"
#include <GL\glew.h>


class GLShader : public ICoreShader
{
	GLuint _programID {0};
	GLuint _vertID {0};
	GLuint _geomID {0};
	GLuint _fragID {0};

public:

	GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID);
	~GLShader();

	GLuint programID() const { return _programID;	}

	API Free() override;
	API GetType(RES_TYPE& type) override;
};

