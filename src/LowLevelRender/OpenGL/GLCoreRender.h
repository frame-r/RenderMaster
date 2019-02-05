#pragma once
#include "Common.h"
#include "gl_objects.inl"


class GLCoreRender final : public ICoreRender
{
	// TODO: move platform stuff from here
	HDC _hdc{};
	HGLRC _hRC{};
	HWND _hWnd{};
	int _pixelFormat = 0;

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
		GLint x = 0, y = 0;
		GLint width = 0, heigth = 0;

		// Shader
		//
		ShaderPtr shader;

		// Mesh
		//
		MeshPtr mesh;

		// Textures
		//
		TexturePtr texShaderBindings[16]; // slot -> texture

		// Framebuffer
		//
		RenderTargetPtr renderTarget;

		// Clear
		//
		GLfloat clearColor[4] = {0.0f, 0.0, 0.0f, 0.0f};
	};

	State _state;
	std::stack<State> _statesStack;
	
	bool checkShaderErrors(int id, GLenum constant);
	bool createShader(GLuint &id, GLenum type, const char* pText, GLuint programID);

public:

	GLCoreRender();
	virtual ~GLCoreRender();

	API_RESULT Init(const WindowHandle* handle, int MSAASamples = 0, int VSyncOn = 1) override;
	API_RESULT Free() override;
	API_RESULT MakeCurrent(const WindowHandle* handle) override;
	API_RESULT SwapBuffers() override;

	API_RESULT CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API_RESULT CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText) override;
	API_RESULT CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API_RESULT CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;
	API_RESULT CreateStructuredBuffer(OUT ICoreStructuredBuffer **pStructuredBuffer, uint size, uint elementSize) override;

	API_VOID PushStates() override;
	API_VOID PopStates() override;

	API_VOID SetCurrentRenderTarget(IRenderTarget *pRenderTarget) override;
	API_VOID RestoreDefaultRenderTarget() override;
	API_VOID BindTexture(uint slot, ITexture* texture) override;
	API_VOID UnbindAllTextures() override;
	API_VOID SetShader(IShader *pShader) override;
	API_VOID SetMesh(IMesh* mesh) override;
	API_VOID SetStructuredBufer(uint slot, IStructuredBuffer* buffer) override;
	API_VOID Draw(IMesh *mesh, uint instances) override;
	API_VOID SetDepthTest(int enabled) override;
	API_VOID SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) override;
	API_VOID SetViewport(uint wIn, uint hIn) override;
	API_VOID GetViewport(OUT uint* wOut, OUT uint* hOut) override;
	API_VOID Clear() override;

	API_VOID ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixel, uint x, uint y) override;
	API_VOID BlitRenderTargetToDefault(IRenderTarget *pRenderTarget) override;

	API_RESULT GetName(OUT const char **pTxt) override;
};

