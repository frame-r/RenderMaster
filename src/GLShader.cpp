#include "GLShader.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

GLShader::GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID) : 
	_programID(programID), _vertID(vertID), _geomID(geomID), _fragID(fragID)
{
}

GLShader::~GLShader()
{
}

API GLShader::Free()
{
	auto free_gl_mesh = [&]() -> void
	{
		if (_vertID != 0) glDeleteShader(_vertID);
		if (_fragID != 0) glDeleteShader(_fragID);
		if (_geomID != 0) glDeleteShader(_geomID);
		if (_programID != 0) glDeleteProgram(_programID);
	};

	standard_free_and_delete(this, free_gl_mesh, _pCore);

	return S_OK;
}

API GLShader::GetType(OUT RES_TYPE *type)
{
	*type = RES_TYPE::CORE_SHADER;
	return S_OK;
}
