#pragma once
#include "common.h"
#include "icorerender.h"

class DX11StructuredBuffer : public ICoreStructuredBuffer
{
	ID3D11Buffer *buf = nullptr;
	ID3D11ShaderResourceView *srv = nullptr;
	uint size = 0;
	uint elementSize = 0;

public:
	DX11StructuredBuffer(ID3D11Buffer *bufIn, ID3D11ShaderResourceView *srvIn);
	virtual ~DX11StructuredBuffer();

	ID3D11ShaderResourceView *SRV() const { return srv; }

	auto SetData(uint8 *data, size_t size) -> void override;
	auto GetSize() -> uint override;
	auto GetElementSize() -> uint override;
	auto GetVideoMemoryUsage() -> size_t override;
};

