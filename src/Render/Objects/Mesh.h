#pragma once
#include "Common.h"

class Mesh : public BaseResource<IMesh>
{
	unique_ptr<ICoreMesh> _coreMesh;

public:
	Mesh(unique_ptr<ICoreMesh> m);
	Mesh(unique_ptr<ICoreMesh> m, const string& filePath);

	API GetCoreMesh(OUT ICoreMesh **meshOut) override;
	API GetNumberOfVertex(OUT uint *number) override;
	API GetAttributes(OUT INPUT_ATTRUBUTE *attribs) override;
	API GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) override;
};
