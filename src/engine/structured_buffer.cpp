#pragma once
#include "pch.h"
#include "structured_buffer.h"

StructuredBuffer::StructuredBuffer(std::unique_ptr<ICoreStructuredBuffer> buf)
{
	_coreStructuredBuffer = std::move(buf);
}

auto DLLEXPORT StructuredBuffer::GetCoreBuffer() -> ICoreStructuredBuffer*
{
	return _coreStructuredBuffer.get();
}

auto DLLEXPORT StructuredBuffer::SetData(uint8 * data, size_t size) -> void
{
	_coreStructuredBuffer->SetData(data, size);
}

auto DLLEXPORT StructuredBuffer::GetVideoMemoryUsage() -> size_t
{
	if (!_coreStructuredBuffer)
		return 0;

	return _coreStructuredBuffer->GetVideoMemoryUsage();
}
