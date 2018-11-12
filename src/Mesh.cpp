#pragma once
#include "Pch.h"
#include "Mesh.h"

API Mesh::GetCoreMesh(OUT ICoreMesh ** coreMeshOut)
{
	*coreMeshOut = _coreMesh;
	return S_OK;
}
