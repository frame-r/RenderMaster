#pragma once
#include "Pch.h"
#include "Core.h"
#include "ConstantBuffer.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(ConstantBuffer, _pCore, RemoveRuntimeConstantBuffer)

API ConstantBuffer::GetCoreBuffer(OUT ICoreConstantBuffer **bufferOut)
{
	*bufferOut = _coreConstantBuffer;
	return S_OK;
}

ConstantBuffer::~ConstantBuffer()
{
	delete _coreConstantBuffer;
	_coreConstantBuffer = nullptr;
}
