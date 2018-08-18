#pragma once
#include "Common.h"
#include <d3d11_2.h>


class DX11Mesh : public ICoreMesh
{
	ID3D11Buffer* _pVertexBuffer{nullptr};
	ID3D11Buffer *_pIndexBuffer = {nullptr};
	ID3D11InputLayout* _pInputLayoyt{nullptr};
	uint _number_of_vertices{0};
	bool _index_presented{false};
	uint _number_of_indicies{0};
	MESH_INDEX_FORMAT _index_format{MESH_INDEX_FORMAT::NOTHING};
	VERTEX_TOPOLOGY _topology{VERTEX_TOPOLOGY::TRIANGLES};
	INPUT_ATTRUBUTE _attributes{INPUT_ATTRUBUTE::CUSTOM};

public:
	
	DX11Mesh(ID3D11Buffer* vb, ID3D11Buffer *ib, ID3D11InputLayout* il, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, VERTEX_TOPOLOGY mode, INPUT_ATTRUBUTE a);
	
	API GetNumberOfVertex(OUT uint *number) override;
	API GetAttributes(OUT INPUT_ATTRUBUTE *attribs) override;
	API GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) override;
	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};
