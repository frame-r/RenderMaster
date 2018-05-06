#pragma once
#include "Common.h"
#include <GL\glew.h>

// TODO: move platform depend stuff from this file
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>


class GLCoreRender : public ICoreRender
{
	HDC _hdc;
	HGLRC _hRC;
	HWND _hWnd;

	const ICoreShader *_current_shader{nullptr};

	IResourceManager *_pResMan{nullptr};

	bool _check_shader_errors(int id, GLenum constant);
	bool _create_shader(GLuint &id, GLenum type, const char** pText, int numLines, GLuint programID);

public:

	GLCoreRender();
	~GLCoreRender();
		
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
	API GetName(const char *&pName) override;
};

