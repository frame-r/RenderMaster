#pragma once
#include "Common.h"

#include <GL\glew.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


class GLCoreRender : public ICoreRender
{
	HDC _hdc;
	HGLRC _hRC;
	HWND _hWnd;

	bool _check_chader_errors(int id, GLenum constant);
	bool _create_shader(GLuint &id, GLenum type, const char** pData, int numLines, GLuint programID);

public:

	GLCoreRender();
	~GLCoreRender();
	
	// ISubSystem
	API GetName(const char *&pTxt) override;

	API Init(WinHandle* handle) override;
	API CreateMesh(ICoreMesh *&pMesh, MeshDataDesc &dataDesc, MeshIndexDesc &indexDesc, DRAW_MODE mode) override;
	API CreateShader(ICoreShader *&pShader, ShaderDesc& shaderDesc) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;
};

