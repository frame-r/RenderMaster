#pragma once
#include "Common.h"


class DX11CoreRender : public ICoreRender
{
public:

	DX11CoreRender();
	~DX11CoreRender();
	
	API Init(const WinHandle* handle) override;
	API CreateMesh(ICoreMesh *&pMesh, const MeshDataDesc &dataDesc, const MeshIndexDesc &indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(ICoreShader *&pShader, const ShaderText& shaderDesc) override;
	API SetShader(const ICoreShader *pShader) override;
	API SetUniform(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type) override;
	API SetUniformArray(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type, uint number) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint w, uint h) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;
	API GetName(const char *&pTxt) override;
};

