#pragma once
#include "Pch.h"
#include "Core.h"
#include "StructuredBuffer.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(StructuredBuffer, _pCore, RemoveRuntimeStructuredBuffer)

StructuredBuffer::StructuredBuffer(unique_ptr<ICoreStructuredBuffer> buf)
{
	_coreStructuredBuffer = std::move(buf);
}

API StructuredBuffer::GetCoreBuffer(ICoreStructuredBuffer **bufOut)
{
	*bufOut = _coreStructuredBuffer.get();
	return S_OK;
}

API StructuredBuffer::SetData(uint8 * data, size_t size)
{
	_coreStructuredBuffer->SetData(data, size);
	return S_OK;
}
