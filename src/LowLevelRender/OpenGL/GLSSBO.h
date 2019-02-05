#pragma once
#include "Common.h"

class GLSSBO : public ICoreStructuredBuffer
{
	GLuint _ID = 0u;
	uint _size = 0u;
	uint _elementSize = 0u;

public:
	GLSSBO(GLuint id, uint sizeIn, uint elementSizeIn);
	virtual ~GLSSBO();

	GLuint ID() const { return _ID; }

	API_RESULT SetData(uint8 *data, size_t size) override;
	API_RESULT GetSize(OUT uint *size) override;
	API_RESULT GetElementSize(OUT uint *size) override;
};

