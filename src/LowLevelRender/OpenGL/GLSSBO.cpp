#include "Pch.h"
#include "GLSSBO.h"
#include "GLCoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


GLSSBO::GLSSBO(GLuint id, uint sizeIn, uint elementSizeIn) : _ID(id), _size(sizeIn), _elementSize(elementSizeIn)
{}

GLSSBO::~GLSSBO()
{
	if (_ID) glDeleteBuffers(1, &_ID);
	_ID = 0u;
	_size = 0u;
	_elementSize = 0u;
}

API GLSSBO::SetData(uint8 *data, size_t size)
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, _ID);
	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(p, data, size);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return S_OK;
}

API GLSSBO::GetSize(OUT uint *sizeOut)
{
	*sizeOut = _size;
	return S_OK;
}

API GLSSBO::GetElementSize(OUT uint *sizeOut)
{
	*sizeOut = _elementSize;
	return S_OK;
}
