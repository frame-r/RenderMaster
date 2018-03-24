#include "GLShader.h"
#include "Core.h"

extern Core *_pCore;

GLShader::GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID) : 
	_programID(programID), _vertID(vertID), _geomID(geomID), _fragID(fragID)
{
}

GLShader::~GLShader()
{
}

API GLShader::Free()
{
	IResourceManager *pResMan;
	uint refNum;

	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	pResMan->GetRefNumber(this, refNum);

	if (refNum == 1)
	{
		pResMan->RemoveFromList(this);

		if (_vertID != 0) glDeleteShader(_vertID);
		if (_fragID != 0) glDeleteShader(_fragID);
		if (_geomID != 0) glDeleteShader(_geomID);
		if (_programID != 0) glDeleteProgram(_programID);
	}
	else
		pResMan->DecrementRef(this);

	return S_OK;
}

API GLShader::GetType(RES_TYPE& type)
{
	type = RES_TYPE::CORE_SHADER;
	return S_OK;
}
