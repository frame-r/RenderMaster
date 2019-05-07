#include "pch.h"
#include "dx11structured_buffer.h"
#include "core.h"
#include "dx11corerender.h"

static ID3D11DeviceContext* getContext()
{
	DX11CoreRender *dxRender = static_cast<DX11CoreRender*>(CORE_RENDER);
	return dxRender->getContext();
}
static ID3D11Device* getDevice()
{
	DX11CoreRender *dxRender = static_cast<DX11CoreRender*>(CORE_RENDER);
	return dxRender->getDevice();
}

DX11StructuredBuffer::DX11StructuredBuffer(ID3D11Buffer * bufIn, ID3D11ShaderResourceView * srvIn) : buf(bufIn), srv(srvIn)
{
	D3D11_BUFFER_DESC desc;
	bufIn->GetDesc(&desc);
	size = desc.ByteWidth;
	elementSize = desc.StructureByteStride;
}

DX11StructuredBuffer::~DX11StructuredBuffer()
{
	if (buf) { buf->Release(); buf = nullptr; }
	if (srv) { srv->Release(); srv = nullptr; }
	size = 0u;
	elementSize = 0u;
}

auto DX11StructuredBuffer::SetData(uint8 *data, size_t size) -> void
{
	ID3D11DeviceContext *ctx = getContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	ctx->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy(mappedResource.pData, data, size);

	ctx->Unmap(buf, 0);
}

auto DX11StructuredBuffer::GetSize() -> uint
{
	return size;
}

auto DX11StructuredBuffer::GetElementSize() -> uint
{
	return elementSize;
}

auto DX11StructuredBuffer::GetVideoMemoryUsage() -> size_t
{
	return size * elementSize;
}
