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
		// Blending
		// Note: blending operation always "+"
		//
		bool blending = false;
		GLenum srcBlend = GL_ONE; // written by shader
		GLenum dstBlend = GL_ZERO; // value in framebuffer

		// Rasterizer
		//
		bool culling = false;
		GLint cullingMode = GL_BACK;	// GL_FRONT, GL_BACK, GL_FRONT_AND_BACK
		GLint polygonMode = GL_FILL;	// GL_FILL, GL_LINE, GL_POINT

		// Depth/Stencil
		//
		bool depthTest = false;

		// Viewport
		//
		GLint viewportX = 0, viewportY = 0;
		GLint viewportWidth = 0, viewportHeigth = 0;

		// Shader
		//
		WRL::ComPtr<IShader> shader;

		// Mesh
		//
		WRL::ComPtr<IMesh> mesh;

		// Textures
		//
		WRL::ComPtr<ITexture> texShaderBindings[16]; // slot -> texture

		// Framebuffer
		//
		WRL::ComPtr<IRenderTarget> renderTarget;

		// Clear
		//
		GLfloat clearColor[4] = {0.0f, 0.0, 0.0f, 0.0f};
	};

	State _state;
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

	API ClearState() override;
	API PushStates() override;
	API PopStates() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText) override;
	API CreateConstantBuffer(OUT ICoreConstantBuffer **pBuffer, uint size) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;

	API SetCurrentRenderTarget(IRenderTarget *pRenderTarget) override;
	API RestoreDefaultRenderTarget() override;
	API SetShader(IShader *pShader) override;
	API SetMesh(IMesh* mesh) override;
	API SetConstantBuffer(IConstantBuffer *pBuffer, uint slot) override;
	API SetConstantBufferData(IConstantBuffer *pBuffer, const void *pData) override;
	API Draw(IMesh *mesh) override;
	API SetDepthState(int enabled) override;
	API SetViewport(uint wIn, uint hIn) override;
	API GetViewport(OUT uint* wOut, OUT uint* hOut) override;
	API Clear() override;

	API ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixel, uint x, uint y) override;

	API GetName(OUT const char **pTxt) override;
};

