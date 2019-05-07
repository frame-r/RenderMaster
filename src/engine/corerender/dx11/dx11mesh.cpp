#include "pch.h"
#include "dx11mesh.h"
#include "core.h"


DX11Mesh::DX11Mesh(ID3D11Buffer* vb, ID3D11Buffer *ib, ID3D11InputLayout* il, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, VERTEX_TOPOLOGY mode, INPUT_ATTRUBUTE a, int bytesWidth):
	_pVertexBuffer(vb), _pIndexBuffer(ib), _pInputLayoyt(il),
	_number_of_vertices(vertexNumber), _number_of_indicies(indexNumber), _index_presented(indexFormat != MESH_INDEX_FORMAT::NONE), _index_format(indexFormat), _topology(mode), _attributes(a), _bytesWidth(bytesWidth)
{
	D3D11_BUFFER_DESC desc;
	_pVertexBuffer->GetDesc(&desc);
	bytes = desc.ByteWidth;

	if (ib)
	{
		_pIndexBuffer->GetDesc(&desc);
		bytes += desc.ByteWidth;
	}
}

DX11Mesh::~DX11Mesh()
{
	if (_pVertexBuffer) _pVertexBuffer->Release();
	if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pInputLayoyt) _pInputLayoyt->Release();
}

auto DX11Mesh::GetVideoMemoryUsage() -> size_t
{
	return bytes;
}

