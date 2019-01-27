#include "Pch.h"
#include "DX11StructuredBuffer.h"
#include "DX11CoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

static ID3D11DeviceContext* getContext(Core *core)
{
	ICoreRender *coreRender = getCoreRender(_pCore);
	DX11CoreRender *dxRender = static_cast<DX11CoreRender*>(coreRender);
	return dxRender->getContext();
}

DX11StructuredBuffer::DX11StructuredBuffer(ID3D11Buffer *bufIn, ID3D11ShaderResourceView *srvIn) : buf(bufIn), srv(srvIn)
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

API DX11StructuredBuffer::SetData(uint8 *data, size_t size)
{
	ID3D11DeviceContext *ctx = getContext(_pCore);

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	ctx->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	memcpy(mappedResource.pData, data, size);

	ctx->Unmap(buf, 0);
	return S_OK;
}

API DX11StructuredBuffer::GetSize(OUT uint *sizeOut)
{
	*sizeOut = size;
	return S_OK;
}

API DX11StructuredBuffer::GetElementSize(OUT uint *sizeOut)
{
	*sizeOut = elementSize;
	return S_OK;
}

