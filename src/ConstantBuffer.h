#pragma once
#include "Common.h"

class ConstantBuffer : public IConstantBuffer
{
	ICoreConstantBuffer *_coreConstantBuffer = nullptr;

public:
	ConstantBuffer(ICoreConstantBuffer *cb) : _coreConstantBuffer(cb) {}
	virtual ~ConstantBuffer() { delete _coreConstantBuffer; _coreConstantBuffer = nullptr; }

	API GetCoreBuffer(OUT ICoreConstantBuffer **bufferOut) override;

	RUNTIME_COM_HEADER_IMPLEMENTATION
};
