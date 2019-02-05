#pragma once
#include "Common.h"

class Mesh : public BaseResource<IMesh>
{
	unique_ptr<ICoreMesh> _coreMesh;

public:
	Mesh(unique_ptr<ICoreMesh> m);
	Mesh(unique_ptr<ICoreMesh> m, const string& filePath);

	API_RESULT GetCoreMesh(OUT ICoreMesh **meshOut) override;
	API_RESULT GetNumberOfVertex(OUT uint *number) override;
	API_RESULT GetAttributes(OUT INPUT_ATTRUBUTE *attribs) override;
	API_RESULT GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) override;
};
