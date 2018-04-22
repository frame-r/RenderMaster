#include "DX11CoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11CoreRender::DX11CoreRender()
{
	LOG("DX11CoreRender initalized");
}

DX11CoreRender::~DX11CoreRender()
{
}

API DX11CoreRender::GetName(const char *& pTxt)
{
	pTxt = "DX11CoreRender";
	return S_OK;
}

API DX11CoreRender::Init(const WinHandle* handle)
{
	return E_NOTIMPL;
}

API DX11CoreRender::CreateMesh(ICoreMesh *&pMesh, const MeshDataDesc &dataDesc, const MeshIndexDesc &indexDesc, DRAW_MODE mode)
{
	return E_NOTIMPL;
}

API DX11CoreRender::CreateShader(ICoreShader *& pShader, const ShaderText & shaderDesc)
{
	return E_NOTIMPL;
}

API DX11CoreRender::Clear()
{
	return E_NOTIMPL;
}

API DX11CoreRender::SwapBuffers()
{
	return E_NOTIMPL;
}

API DX11CoreRender::Free()
{
	return E_NOTIMPL;
}
