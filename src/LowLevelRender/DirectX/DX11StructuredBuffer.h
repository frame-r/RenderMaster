#pragma once
#include "Common.h"

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

	API SetData(uint8 *data, size_t size) override;
	API GetSize(OUT uint *size) override;
	API GetElementSize(OUT uint *size) override;
};

