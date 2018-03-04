#pragma once
#include "Common.h"


class Mesh : public IMesh
{
	uint _vertex;

public:

	Mesh();

	API GetVertexCount(uint &vertex) override;

	virtual void Free();
};