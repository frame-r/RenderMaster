#pragma once
#include "Common.h"

class Mesh : public IMesh
{
	ICoreMesh *_coreMesh = nullptr;

public:
	Mesh(ICoreMesh *m) : _coreMesh(m) {}
	Mesh(ICoreMesh *m, const string& filePath) : _coreMesh(m),_file(filePath) {}
	virtual ~Mesh(); 

	API GetCoreMesh(OUT ICoreMesh **meshOut) override;
	API GetNumberOfVertex(OUT uint *number) override;
	API GetAttributes(OUT INPUT_ATTRUBUTE *attribs) override;
	API GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) override;

	BASE_RESOURCE_HEADER
};
