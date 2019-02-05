#pragma once
#include "Pch.h"
#include "Core.h"
#include "Mesh.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

API Mesh::GetCoreMesh(OUT ICoreMesh ** coreMeshOut)
{
	*coreMeshOut = _coreMesh.get();
	return S_OK;
}

Mesh::Mesh(unique_ptr<ICoreMesh> m)
{
	_coreMesh = std::move(m);
}

Mesh::Mesh(unique_ptr<ICoreMesh> m, const string& filePath)
{
	_coreMesh = std::move(m);
	_file = filePath;
}

API Mesh::GetNumberOfVertex(OUT uint *number)
{
	return _coreMesh->GetNumberOfVertex(number);
}

API Mesh::GetAttributes(OUT INPUT_ATTRUBUTE *attribs)
{
	return _coreMesh->GetAttributes(attribs);
}

API Mesh::GetVertexTopology(OUT VERTEX_TOPOLOGY *topology)
{
	return _coreMesh->GetVertexTopology(topology);
}
