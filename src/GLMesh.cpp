#include "GLMesh.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

GLMesh::GLMesh(GLuint VAO, GLuint VBO, GLuint IBO, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, VERTEX_TOPOLOGY mode, INPUT_ATTRUBUTE a):
	_VAO(VAO), _VBO(VBO), _IBO(IBO),
	_number_of_vertices(vertexNumber), _number_of_indicies(indexNumber), _index_presented(indexFormat != MESH_INDEX_FORMAT::NOTHING), _topology(mode), _attributes(a)
{
}

API GLMesh::GetNumberOfVertex(uint & vertex)
{
	vertex = _number_of_vertices;
	return S_OK;
}

API GLMesh::GetAttributes(INPUT_ATTRUBUTE& attribs)
{
	attribs = _attributes;
	return S_OK;
}

API GLMesh::GetVertexTopology(VERTEX_TOPOLOGY& topology)
{
	topology = _topology;
	return S_OK;
}

API GLMesh::Free()
{
	IResourceManager *pResMan;
	_pCore->GetSubSystem((ISubSystem*&)pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	uint refNum;
	pResMan->GetRefNumber(this, refNum);

	if (refNum == 1)
	{
		pResMan->RemoveFromList(this);

		if (_index_presented) glDeleteBuffers(1, &_IBO);
		glDeleteBuffers(1, &_VBO);
		glDeleteVertexArrays(1, &_VAO);
	}
	else if (refNum > 1)
		pResMan->DecrementRef(this);
	else
		LOG_WARNING("GLMesh::Free(): refNum == 0");

	delete this;

	return S_OK;
}

API GLMesh::GetType(RES_TYPE& type)
{
	type = RES_TYPE::CORE_MESH;
	return S_OK;
}
