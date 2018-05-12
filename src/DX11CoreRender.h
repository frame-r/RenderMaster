#pragma once
#include "Common.h"


class DX11CoreRender final : public ICoreRender
{
public:

	DX11CoreRender();
	~DX11CoreRender();
	
	API Init(const WinHandle* handle) override;
	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc) override;
	API SetShader(const ICoreShader *pShader) override;
	API SetUniform(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type) override;
	API SetUniformArray(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type, uint number) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API MakeCurrent(const WinHandle* handle) override;
	API SetViewport(uint w, uint h) override;
	API GetViewport(OUT uint* w, OUT uint* h) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;
	API GetName(OUT const char **pTxt) override;
};

