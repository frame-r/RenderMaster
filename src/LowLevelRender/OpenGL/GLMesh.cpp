#include "Pch.h"
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

GLMesh::~GLMesh()
{
	if (_index_presented) {	glDeleteBuffers(1, &_IBO); _IBO = 0; }
	if (_VBO) {	glDeleteBuffers(1, &_VBO); _VBO = 0; }
	if (_VAO) {	glDeleteVertexArrays(1, &_VAO); _VAO = 0; }
}

API_RESULT GLMesh::GetNumberOfVertex(OUT uint *vertex)
{
	*vertex = _number_of_vertices;
	return S_OK;
}

API_RESULT GLMesh::GetAttributes(OUT INPUT_ATTRUBUTE *attribs)
{
	*attribs = _attributes;
	return S_OK;
}

API_RESULT GLMesh::GetVertexTopology(OUT VERTEX_TOPOLOGY *topology)
{
	*topology = _topology;
	return S_OK;
}

