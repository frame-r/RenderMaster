#pragma once
#include "Common.h"

class GLMesh : public ICoreMesh
{
	GLuint _VAO = 0u;
	GLuint _VBO = 0u;
	GLuint _IBO = 0u;
	uint _number_of_vertices = 0u;
	bool _index_presented = false;
	uint _number_of_indicies = 0u;
	MESH_INDEX_FORMAT _index_format = MESH_INDEX_FORMAT::NOTHING;
	VERTEX_TOPOLOGY _topology = VERTEX_TOPOLOGY::TRIANGLES;
	INPUT_ATTRUBUTE _attributes = INPUT_ATTRUBUTE::UNKNOWN;

public:
	
	GLMesh(GLuint VAO, GLuint VBO, GLuint IBO, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, VERTEX_TOPOLOGY mode, INPUT_ATTRUBUTE a);
	virtual ~GLMesh();

	GLuint VAO_ID() const { return _VAO; }
	uint Indexes() { return _number_of_indicies; }

	API GetNumberOfVertex(OUT uint *number) override;
	API GetAttributes(OUT INPUT_ATTRUBUTE *attribs) override;
	API GetVertexTopology(OUT VERTEX_TOPOLOGY *topology) override;
};
