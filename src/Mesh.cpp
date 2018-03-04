#include "Mesh.h"

Mesh::Mesh() : _vertex(0)
{
}

API Mesh::GetVertexCount(uint & vertex)
{
	vertex = _vertex;

	return S_OK;
}

void Mesh::Free()
{
}
