#pragma once
#include "Common.h"
#include <GL\glew.h>


class GLMesh : public ICoreMesh
{
	GLuint _VAO {0};
	GLuint _VBO {0};
	GLuint _IBO {0};
	uint _number_of_vertices{0};
	bool _index_presented{false};
	uint _number_of_indicies{0};
	MESH_INDEX_FORMAT _index_format{MESH_INDEX_FORMAT::NOTHING};
	DRAW_MODE _mode{DRAW_MODE::TRIANGLES};
	INPUT_ATTRUBUTE _attributes{INPUT_ATTRUBUTE::NONE};

public:
	
	GLMesh(GLuint VAO, GLuint VBO, GLuint IBO, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, DRAW_MODE mode, INPUT_ATTRUBUTE a);

	GLuint VAO_ID() const { return _VAO; }

	API GetNumberOfVertex(uint &number) override;
	API GetAttributes(INPUT_ATTRUBUTE &attribs) override;
	API Free() override;
	API GetType(RES_TYPE& type) override;
};
