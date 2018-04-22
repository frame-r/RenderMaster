#pragma once
#include "Common.h"


class DX11CoreRender : public ICoreRender
{
public:

	DX11CoreRender();
	~DX11CoreRender();
	
	API Init(const WinHandle* handle) override;
	API CreateMesh(ICoreMesh *&pMesh, const MeshDataDesc &dataDesc, const MeshIndexDesc &indexDesc, DRAW_MODE mode) override;
	API CreateShader(ICoreShader *&pShader, const ShaderText& shaderDesc) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;
	API GetName(const char *&pTxt) override;
};

