#pragma once
#include "Common.h"

class DX11CoreRender : public ICoreRender
{
public:

	DX11CoreRender();
	~DX11CoreRender();

	// ISubSystem
	API GetName(const char *&pTxt) override;

	API Init(WinHandle* handle) override;
	API CreateMesh(ICoreMesh *&pMesh, MeshDataDesc &dataDesc, MeshIndexDesc &indexDesc, DRAW_MODE mode) override;
	API CreateShader(ICoreShader *&pShader, ShaderDesc& shaderDesc) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;

};

