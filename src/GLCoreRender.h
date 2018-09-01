#pragma once
#include "Common.h"
#include <GL\glew.h>

// TODO: move platform depend stuff from this file
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>


class GLUniformBuffer final : public IUniformBuffer
{
	GLuint _UBO;
	uint _size{0};

public:

	GLUniformBuffer(GLuint ID, uint size) : _UBO(ID), _size(size) {}

	inline GLuint ID() const { return _UBO; }
	inline uint size() const { return _size; }

	API Free() override;
	API GetType(OUT RES_TYPE *type) override;
};


class GLCoreRender final : public ICoreRender
{
	HDC _hdc{};
	HGLRC _hRC{};
	HWND _hWnd{};
	int pixel_format{0};
	IResourceManager *_pResMan{nullptr};

	struct State
	{
		//
		// Blending
		// Note: blending operation always "+"
		//
		int blending_enabled{0};
		GLenum src_blend_factor{GL_ZERO}; // written by shader
		GLenum dst_blend_factor{GL_ZERO}; // value in framebuffer

		//
		// Shader
		//
		GLuint shader_program_id{0u};

		//
		// Textures
		//
		struct SlotBindingDesc
		{
			GLuint tex_id{0u};
			GLuint shader_var_id{0u};			
		};
		SlotBindingDesc tex_slots_bindings[16]; // slot -> binded to slot shader variable and texture

		//
		// Rasterizer state
		//
		GLfloat clear_color[4] = {0.0f, 0.0, 0.0f, 1.0f};
		GLint poligon_mode{GL_FILL};				// GL_FILL	GL_LINE
		GLboolean culling_enabled{GL_FALSE};		// GL_FALSE	GL_TRUE
		GLint culling_mode{GL_FRONT};				// GL_FRONT	GL_BACK
		GLboolean depth_test_enabled{GL_FALSE};

		//
		// Viewport
		//
		GLint viewport_x{0}, viewport_y{0};
		GLint viewport_w{0}, viewport_h{0};
	};
	State _current_state;
	std::stack<State> _states;
	
	bool _check_shader_errors(int id, GLenum constant);
	bool _create_shader(GLuint &id, GLenum type, const char* pText, GLuint programID);

public:

	GLCoreRender();
	virtual ~GLCoreRender();

	API Init(const WinHandle* handle) override;
	API PushStates() override;
	API PopStates() override;
	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc) override;
	API SetShader(const ICoreShader *pShader) override;
	API CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size) override;
	API SetUniform(IUniformBuffer *pBuffer, const void *pData) override;
	API SetUniformBufferToShader(IUniformBuffer *pBuffer, uint slot) override;
	API SetUniform(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type) override;
	API SetUniformArray(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type, uint number) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API MakeCurrent(const WinHandle* handle) override;
	API SetViewport(uint wIn, uint hIn) override;
	API GetViewport(OUT uint* wOut, OUT uint* hOut) override;
	API Clear() override;
	API SwapBuffers() override;
	API Free() override;
	API GetName(OUT const char **pTxt) override;
};

