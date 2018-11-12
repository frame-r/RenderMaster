#pragma once
#include "Common.h"

class Mesh : public IMesh
{
	ICoreMesh *_coreMesh = nullptr;

public:
	Mesh(ICoreMesh *m) : _coreMesh(m) {}
	Mesh(ICoreMesh *m, int isShared, const string& fileIn) : _coreMesh(m), _isShared(isShared), _file(fileIn) {}
	virtual ~Mesh() { delete _coreMesh; _coreMesh = nullptr; }

	API GetCoreMesh(OUT ICoreMesh **meshOut) override;

	BASE_COM_HEADER_IMPLEMENTATION
};
