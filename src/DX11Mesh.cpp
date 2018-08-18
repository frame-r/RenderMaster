#include "DX11Mesh.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11Mesh::DX11Mesh(ID3D11Buffer* vb, ID3D11Buffer *ib, ID3D11InputLayout* il, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, VERTEX_TOPOLOGY mode, INPUT_ATTRUBUTE a):
	_pVertexBuffer(vb), _pIndexBuffer(ib), _pInputLayoyt(il),
	_number_of_vertices(vertexNumber), _number_of_indicies(indexNumber), _index_presented(indexFormat != MESH_INDEX_FORMAT::NOTHING), _index_format(indexFormat), _topology(mode), _attributes(a)
{
}

API DX11Mesh::GetNumberOfVertex(OUT uint *vertex)
{
	*vertex = _number_of_vertices;
	return S_OK;
}

API DX11Mesh::GetAttributes(OUT INPUT_ATTRUBUTE *attribs)
{
	*attribs = _attributes;
	return S_OK;
}

API DX11Mesh::GetVertexTopology(OUT VERTEX_TOPOLOGY *topology)
{
	*topology = _topology;
	return S_OK;
}

API DX11Mesh::Free()
{
	auto free_dx_mesh = [&]() -> void
	{
		if (_pVertexBuffer) _pVertexBuffer->Release();
		if (_pIndexBuffer) _pIndexBuffer->Release();
		if (_pInputLayoyt) _pInputLayoyt->Release();
	};

	standard_free_and_delete(this, free_dx_mesh, _pCore);

	return S_OK;
}

API DX11Mesh::GetType(OUT RES_TYPE *type)
{
	*type = RES_TYPE::CORE_MESH;
	return S_OK;
}
