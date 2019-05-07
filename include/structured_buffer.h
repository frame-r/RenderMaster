#pragma once
#include "common.h"
#include "icorerender.h"

class StructuredBuffer
{
	std::unique_ptr<ICoreStructuredBuffer> _coreStructuredBuffer;

public:
	StructuredBuffer(std::unique_ptr<ICoreStructuredBuffer> buf);

	auto DLLEXPORT GetCoreBuffer()-> ICoreStructuredBuffer*;
	auto DLLEXPORT SetData(uint8 *data, size_t size) -> void;
	auto DLLEXPORT GetVideoMemoryUsage() -> size_t;
};
