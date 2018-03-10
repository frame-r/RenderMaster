#include "GLMesh.h"
#include "Core.h"

extern Core *_pCore;

GLMesh::GLMesh(GLuint VAO, GLuint VBO, GLuint IBO, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, DRAW_MODE mode):
	_VAO(VAO), _VBO(VBO), _IBO(IBO),
	_vertex(vertexNumber), _index(indexNumber), _indexPresented(indexFormat != MESH_INDEX_FORMAT::MID_NOTHING), _mode(mode)
{
}

API GLMesh::GetNumberOfVertex(uint & vertex)
{
	vertex = _vertex;

	return S_OK;
}

API GLMesh::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::ST_RESOURCE_MANAGER);

	uint refNum;
	pResMan->GetRefNumber(this, refNum);

	if (refNum == 1)
	{
		pResMan->RemoveFromList(this);

		if (_indexPresented) glDeleteBuffers(1, &_IBO);
		glDeleteBuffers(1, &_VBO);
		glDeleteVertexArrays(1, &_VAO);
	}
	else
		pResMan->DecrementRef(this);

	return S_OK;
}

API GLMesh::GetType(RES_TYPE& type)
{
	type = RES_TYPE::RT_CORE_MESH;
	return S_OK;
}
