#include "pch.h"
#include "GLTexture.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

GLTexture::GLTexture(GLuint idIn, TEXTURE_FORMAT formatIn)
 : _textureID(idIn), _format(formatIn)
{
	int w, h;
	glBindTexture(GL_TEXTURE_2D, _textureID);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, 0);
	_width = w;
	_height = h;
}

GLTexture::~GLTexture()
{
	if (_textureID)
		glDeleteTextures(1, &_textureID);
}

