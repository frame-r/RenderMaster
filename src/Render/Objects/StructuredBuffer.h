#pragma once
#include "Common.h"

class StructuredBuffer : public BaseResource<IStructuredBuffer>
{
	unique_ptr<ICoreStructuredBuffer> _coreStructuredBuffer;

public:
	StructuredBuffer(unique_ptr<ICoreStructuredBuffer> buf);

	API_RESULT GetCoreBuffer(ICoreStructuredBuffer **bufOut) override;
	API_RESULT SetData(uint8 *data, size_t size) override;
};
