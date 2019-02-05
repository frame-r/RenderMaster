#pragma once
#include "Common.h"

class StructuredBuffer : public BaseResource<IStructuredBuffer>
{
	unique_ptr<ICoreStructuredBuffer> _coreStructuredBuffer;

public:
	StructuredBuffer(unique_ptr<ICoreStructuredBuffer> buf);

	API GetCoreBuffer(ICoreStructuredBuffer **bufOut) override;
	API SetData(uint8 *data, size_t size) override;
};
