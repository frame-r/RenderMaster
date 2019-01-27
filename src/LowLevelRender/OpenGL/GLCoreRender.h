#pragma once
#include "Common.h"

struct UBO final
{
	string name;
	size_t bytes = 0u;
	bool needFlush = true;
	GLuint ID = 0u;
	unique_ptr<uint8[]> data;

	struct Parameter
	{
		string name;
		size_t offset = 0u;
		size_t bytes = 0u;
		size_t elements = 1u; // number of elements in array (if parameter is array)
	};
	vector<Parameter> parameters;

public:

	UBO(GLuint IDIn, size_t bytesIn, const string& nameIn, const vector<Parameter>& paramsIn) :
		ID(IDIn), bytes(bytesIn), name(nameIn), parameters(paramsIn)
	{
		data = std::make_unique<uint8[]>(bytesIn);
		memset(data.get(), 0, bytesIn);
	}

	UBO(const UBO& r) = delete;

	UBO(UBO&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		ID = r.ID;
		r.ID = 0u;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}

	UBO& operator=(UBO&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		ID = r.ID;
		r.ID = 0u;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}

	UBO& operator=(const UBO& r) = delete;

	~UBO()
	{
		if (ID) { glDeleteBuffers(1, &ID);
		ID = 0u; }
	}
};

class GLRenderTarget final : public ICoreRenderTarget
{
	GLuint _ID = 0u;
	GLuint _colors[8];
	GLuint _depth = 0u;

public:
	GLRenderTarget(GLuint idIn);
	virtual ~GLRenderTarget(){ if (_ID) glDeleteFramebuffers(1, &_ID); _ID = 0; }

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

	API Init(const WindowHandle* handle, int MSAASamples = 0, int VSyncOn = 1) override;
	API Free() override;
	API MakeCurrent(const WindowHandle* handle) override;
	API SwapBuffers() override;

	API CreateMesh(OUT ICoreMesh **pMesh, const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) override;
	API CreateShader(OUT ICoreShader **pShader, const char *vertText, const char *fragText, const char *geomText) override;
	API CreateTexture(OUT ICoreTexture **pTexture, uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) override;
	API CreateRenderTarget(OUT ICoreRenderTarget **pRenderTarget) override;
	API CreateStructuredBuffer(OUT ICoreStructuredBuffer **pStructuredBuffer, uint size, uint elementSize) override;

	API PushStates() override;
	API PopStates() override;
	API SetCurrentRenderTarget(IRenderTarget *pRenderTarget) override;
	API RestoreDefaultRenderTarget() override;
	API BindTexture(uint slot, ITexture* texture) override;
	API UnbindAllTextures() override;
	API SetShader(IShader *pShader) override;
	API SetMesh(IMesh* mesh) override;
	API SetStructuredBufer(uint slot, IStructuredBuffer* buffer) override;
	API Draw(IMesh *mesh, uint instances) override;
	API SetDepthTest(int enabled) override;
	API SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) override;
	API SetViewport(uint wIn, uint hIn) override;
	API GetViewport(OUT uint* wOut, OUT uint* hOut) override;
	API Clear() override;

	API ReadPixel2D(ICoreTexture *tex, OUT void *out, OUT uint* readPixel, uint x, uint y) override;
	API BlitRenderTargetToDefault(IRenderTarget *pRenderTarget) override;

	API GetName(OUT const char **pTxt) override;
};

