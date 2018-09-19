#include "DX11Shader.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11Shader::~DX11Shader()
{
	Free();
}

void DX11Shader::Free()
{
	if (pVertex) { pVertex->Release(); pVertex = nullptr;	}
	if (pFragment) { pFragment->Release(); pFragment = nullptr; }
	if (pGeometry) { pGeometry->Release(); pGeometry = nullptr; }
}

