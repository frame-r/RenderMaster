#pragma once
#include "Common.h"

class DX11Mesh : public ICoreMesh
{
	ID3D11Buffer* _pVertexBuffer = nullptr;
	ID3D11Buffer *_pIndexBuffer = nullptr;
	ID3D11InputLayout* _pInputLayoyt = nullptr;
	uint _number_of_vertices = 0u;
	bool _index_presented = false;
	uint _number_of_indicies = 0;
	MESH_INDEX_FORMAT _index_format = MESH_INDEX_FORMAT::NOTHING;
	VERTEX_TOPOLOGY _topology = VERTEX_TOPOLOGY::TRIANGLES;
	INPUT_ATTRUBUTE _attributes = INPUT_ATTRUBUTE::UNKNOWN;
	int _bytesWidth = 0;

public:

	DX11Mesh(ID3D11Buffer* vb, ID3D11Buffer *ib, ID3D11InputLayout* il, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, VERTEX_TOPOLOGY mode, INPUT_ATTRUBUTE a, int bytesWidth);
	virtual ~DX11Mesh();

	ID3D11Buffer *		indexBuffer() const { return _pIndexBuffer; }
	ID3D11Buffer*		vertexBuffer() const { return _pVertexBuffer; }
	ID3D11InputLayout*	inputLayout() const { return _pInputLayoyt; }
	int					stride() const { return _bytesWidth; }
	UINT				vertexNumber() const { return _number_of_vertices; }
	MESH_INDEX_FORMAT	indexFormat() const { return _index_format; }
	UINT				indexNumber() const { return _number_of_indicies; }
	VERTEX_TOPOLOGY		topology() const { return _topology; }

	API_RESULT GetNumberOfVertex(OUT uint *number) override;
	API_RESULT GetAttributes(OUT INPUT_ATTRUBUTE *attribs) override;
	API_RESULT GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) override;
};
