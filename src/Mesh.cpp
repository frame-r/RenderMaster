#pragma once
#include "Pch.h"
#include "Core.h"
#include "Mesh.h"
#include "ResourceManager.h"

extern Core *_pCore;

BASE_COM_CPP_IMPLEMENTATION(Mesh, _pCore, RemoveRuntimeMesh, RemoveSharedMesh)

API Mesh::GetCoreMesh(OUT ICoreMesh ** coreMeshOut)
{
	*coreMeshOut = _coreMesh;
	return S_OK;
}
