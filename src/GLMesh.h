#pragma once
#include "Common.h"
#include <GL\glew.h>


class GLMesh : public ICoreMesh
{
	GLuint _VAO;
	GLuint _VBO;
	GLuint _IBO;
	uint _vertex;
	uint _index;
	bool _indexPresented;
	MESH_INDEX_FORMAT _indexFormat;
	DRAW_MODE _mode;

public:
	
	GLMesh(GLuint VAO, GLuint VBO, GLuint IBO, uint vertexNumber, uint indexNumber, MESH_INDEX_FORMAT indexFormat, DRAW_MODE mode);

	API GetNumberOfVertex(uint &number) override;
	API Free() override;
	API GetType(RES_TYPE& type) override;
};