#pragma once
#include "Common.h"
#include "Mesh.h"
#include <vector>

class ResourceManager : public IResourceManager
{
	CRITICAL_SECTION _cs;
	std::vector<Mesh *> _mesh_vec;

public:

	ResourceManager();
	~ResourceManager();

	// ISubSystem
	API GetName(const char *&pTxt) override;
	
	// IResourceManager
	API LoadMesh(IMesh *&pMesh, const char *pFileName, IProgressSubscriber *pPregress) override;
};