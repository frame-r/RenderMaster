#pragma once
#include "Common.h"


class GLUniformBuffer final : public ICoreConstantBuffer
{
	GLuint _ID = 0u;
	uint _size = 0u;

public:
	GLUniformBuffer(GLuint ID, uint size) : _ID(ID), _size(size) {}
	virtual ~GLUniformBuffer(); 

	inline GLuint ID() const { return _ID; }
	inline uint size() const { return _size; }
};

class GLRenderTarget : public ICoreRenderTarget
{
	GLuint _ID = 0u;
	GLuint _colors[8];
	GLuint _depth = 0u;

public:
	GLRenderTarget(GLuint idIn);
	virtual ~GLRenderTarget(){ if (_ID) glDeleteFramebuffers(1, &_ID); }

	GLuint ID() { return _ID; }

	void bind();

	API SetColorTexture(uint slot, ITexture *tex) override;
	API SetDepthTexture(ITexture *tex) override;
	API UnbindColorTexture(uint slot) override;
	API UnbindAll() override;
};


class GLCoreRender final : public ICoreRender
{
	HDC _hdc{};
	HGLRC _hRC{};
	HWND _hWnd{};
	int _pixel_format = 0;
	IResourceManager *_pResMan = nullptr;

	struct State
	{
		//
		// Blending
		// Note: blending operation always "+"
		//
		int blending_enabled = 0;
		GLenum src_blend_factor = GL_ZERO; // written by shader
		GLenum dst_blend_factor = GL_ZERO; // value in framebuffer

		//
		// Shader
		//
		GLuint shader_program_id = 0u;

		//
		// Textures
		//
		struct SlotBindingDesc
		{
			GLuint tex_id = 0u;
			GLuint shader_variable_id = 0u;			
		};
		SlotBindingDesc tex_slots_bindings[16]; // slot -> {shader, texture}

		//
		// Rasterizer state
		//
		GLfloat clear_color[4] = {0.0f, 0.0, 0.0f, 1.0f};
		GLint poligon_mode = GL_FILL;				// GL_FILL	GL_LINE
		GLboolean culling_enabled = GL_FALSE;		// GL_FALSE	GL_TRUE
		GLint culling_mode = GL_FRONT;				// GL_FRONT	GL_BACK
		GLboolean depth_test_enabled = GL_FALSE;

		//
		// Viewport
		//
		GLint viewport_x = 0, viewport_y = 0;
		GLint viewport_w = 0, viewport_h = 0;

		//
		// Framebuffer
		//
		GLint fbo = 0u; // 0 - default FBO
	};
	State _currentState;
	std::stack<State> _statesStack;
	
	bool check_shader_errors(int id, GLenum constant);
	bool create_shader(GLuint &id, GLenum type, const char* pText, GLuint programID);

public:

	GLCoreRender();
	virtual ~GLCoreRender();

	API Init(const WindowHandle* handle, int MSAASamples = 0, int VSyncOn = 1) override;
	API Free() override;
	API MakeCurrent(const WindowHandle* handle) override;
	API SwapBuffers() override;

	API PushStates() override;
	API PopStates() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const char *vert, const char *frag, const char *geom) override;
	API CreateConstantBuffer(OUT ICoreConstantBuffer **pBuffer, uint size) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;

	API SetCurrentRenderTarget(ICoreRenderTarget *pRenderTarget) override;
	API RestoreDefaultRenderTarget() override;
	API ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixel, uint x, uint y) override;
	API SetShader(const ICoreShader *pShader) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API SetConstantBuffer(const ICoreConstantBuffer *pBuffer, uint slot) override;
	API SetConstantBufferData(ICoreConstantBuffer *pBuffer, const void *pData) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint wIn, uint hIn) override;
	API GetViewport(OUT uint* wOut, OUT uint* hOut) override;
	API Clear() override;

	API GetName(OUT const char **pTxt) override;
};

