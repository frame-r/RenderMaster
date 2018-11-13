#pragma once
#include "Pch.h"
#include "Core.h"
#include "ConstantBuffer.h"
#include "ResourceManager.h"

extern Core *_pCore;

RUNTIME_COM_CPP_IMPLEMENTATION(ConstantBuffer, _pCore, RemoveRuntimeConstantBuffer)

API ConstantBuffer::GetCoreBuffer(OUT ICoreConstantBuffer **bufferOut)
{
	*bufferOut = _coreConstantBuffer;
	return S_OK;
}
