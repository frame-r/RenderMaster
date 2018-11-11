#pragma once
#include "Common.h"


class GLUniformBuffer final : public IUniformBuffer
{
	GLuint _UBO;
	uint _size{0};

public:

	GLUniformBuffer(GLuint ID, uint size) : _UBO(ID), _size(size) {}
	~GLUniformBuffer() { Free(); }

	API Free();

	inline GLuint ID() const { return _UBO; }
	inline uint size() const { return _size; }
};


class GLCoreRender final : public ICoreRender
{
	HDC _hdc{};
	HGLRC _hRC{};
	HWND _hWnd{};
	int _pixel_format{0};
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
			GLuint shader_variable_id{0u};			
		};
		SlotBindingDesc tex_slots_bindings[16]; // slot -> {shader, texture}

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
	State _currentState;
	std::stack<State> _statesStack;
	
	bool check_shader_errors(int id, GLenum constant);
	bool create_shader(GLuint &id, GLenum type, const char* pText, GLuint programID);

public:

	GLCoreRender();
	virtual ~GLCoreRender();

	API Init(const WindowHandle* handle, int MSAASamples = 0) override;
	API Free() override;
	API MakeCurrent(const WindowHandle* handle) override;
	API SwapBuffers() override;

	API PushStates() override;
	API PopStates() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const ShaderText *shaderDesc) override;
	API CreateUniformBuffer(OUT IUniformBuffer **pBuffer, uint size) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API SetShader(const ICoreShader *pShader) override;
	API SetMesh(const ICoreMesh* mesh) override;
	API SetUniformBuffer(const IUniformBuffer *pBuffer, uint slot) override;
	API SetUniformBufferData(IUniformBuffer *pBuffer, const void *pData) override;
	API Draw(ICoreMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint wIn, uint hIn) override;
	API GetViewport(OUT uint* wOut, OUT uint* hOut) override;
	API Clear() override;

	API GetName(OUT const char **pTxt) override;
};

