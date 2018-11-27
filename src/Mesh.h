#pragma once
#include "Common.h"

class Mesh : public IMesh
{
	ICoreMesh *_coreMesh = nullptr;

public:
	Mesh(ICoreMesh *m) : _coreMesh(m) {}
	Mesh(ICoreMesh *m, const string& filePath) : _coreMesh(m),_file(filePath) {}
	virtual ~Mesh(); 

	API GetCoreMesh(OUT ICoreMesh **meshOut) override;

	BASE_RESOURCE_HEADER
};
