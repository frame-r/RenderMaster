#pragma once
#include "Common.h"

struct RenderBuffers
{
	uint height;
	uint width;

	WRL::ComPtr<ITexture> color;		// RGBA8	- Result tonemaped frame

	WRL::ComPtr<ITexture> colorHDR;		// RGBA16F	- HDR frame
	WRL::ComPtr<ITexture> depth;		// D24S8	- Hardware depth

	WRL::ComPtr<ITexture> directLight;	// RGB16F	- Accumulation texture for all lights	

	// GBuffer
	WRL::ComPtr<ITexture> normal;		// RGB8		- World space normal
	WRL::ComPtr<ITexture> shading;		// RGB8		- ?

	WRL::ComPtr<ITexture> id;			// R32UI	- Models id
};

//
// Hight-lever render
// Based on CoreRender (GLCoreRender or DX11CoreRender)
//
class Render : public IRender
{
	ICoreRender *_pCoreRender{nullptr};
	IResourceManager *_pResMan{nullptr};
	ISceneManager *_pSceneMan{nullptr};
	IFileSystem *_fsystem{nullptr};

	WRL::ComPtr<IShaderFile> _forwardShader;
	WRL::ComPtr<IShaderFile> _postShader;
	WRL::ComPtr<IShaderFile> _idShader;	
	WRL::ComPtr<IMesh> _postPlane;
	WRL::ComPtr<IRenderTarget> renderTarget;

	struct TexturePoolable
	{
		int64_t frame;
		int free;
		uint width;
		uint height;
		TEXTURE_FORMAT format;
		WRL::ComPtr<ITexture> tex;
	};
	vector<TexturePoolable> _texture_pool;		

	std::unordered_map<ShaderRequirement, WRL::ComPtr<IShader>, ShaderRequirement> _shaders_pool;
	
	struct RenderMesh
	{
		uint model_id;
		IMesh *mesh{nullptr};
		mat4 modelMat;
	};

	void renderForward(RenderBuffers& buffers, const mat4& V, const mat4& VP, vector<RenderMesh>& meshes);
	void renderEnginePost(RenderBuffers& buffers, const mat4& V, const mat4& VP, vector<RenderMesh>& meshes);

	void _update();
	void setShaderParameters(const mat4& V, const mat4& VP,RenderMesh *mesh, RENDER_PASS pass, IShader *shader);
	IShader* getShader(const ShaderRequirement &req);
	bool isOpenGL();
	void getRenderMeshes(vector<RenderMesh>& meshes);
	void drawMeshes(const mat4& V, const mat4& VP, vector<RenderMesh>& meshes, RENDER_PASS pass);
	ITexture* getRenderTargetTexture2d(uint width, uint height, TEXTURE_FORMAT format);
	void releaseTexture2d(ITexture *tex);
	RenderBuffers initBuffers(uint w, uint h);
	void releaseBuffers(RenderBuffers& buffers);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void Init();
	void Free();
	void RenderFrame(const ICamera *pCamera);

	API PreprocessStandardShader(OUT IShader **pShader, const ShaderRequirement *shaderReq) override;
	API RenderPassIDPass(const ICamera *pCamera, ITexture *tex, ITexture *depthTex) override;
	API GetRenderTexture2D(OUT ITexture **texOut, uint width, uint height, TEXTURE_FORMAT format) override;
	API ReleaseRenderTexture2D(ITexture *texIn) override;
	API ShadersReload() override;
	API GetName(OUT const char **pName) override;
};

