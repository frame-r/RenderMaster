#pragma once
#include "common.h"
#include "icorerender.h"

class DX11StructuredBuffer : public ICoreStructuredBuffer
{
	ID3D11Buffer *buf = nullptr;
	ID3D11ShaderResourceView *srv = nullptr;
	uint size = 0;
	uint elementSize = 0;
	BUFFER_USAGE usage;

public:
	DX11StructuredBuffer(ID3D11Buffer *buf_, ID3D11ShaderResourceView *srv_, BUFFER_USAGE usage_);
	virtual ~DX11StructuredBuffer();

	ID3D11ShaderResourceView *SRV() const { return srv; }

	auto SetData(uint8 *data, size_t size) -> void override;
	auto GetSize() -> uint override;
	auto GetElementSize() -> uint override;
	auto GetVideoMemoryUsage() -> size_t override;
};

