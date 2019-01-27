#pragma once
#include "Common.h"

struct RenderBuffers
{
	uint height;
	uint width;

	TexturePtr color;		// RGBA8	- Result tonemaped frame
	TexturePtr colorHDR;	// RGBA16F	- HDR frame
	TexturePtr depth;		// D24S8	- Hardware depth

	// GBuffer
	TexturePtr normal;		// RGB8		- World space normal
	TexturePtr shading;		// RGB8		- ?

	TexturePtr directLight;	// RGB16F	- Accumulation texture for all lights
	TexturePtr id;			// R32UI	- Models id
};

//
// Hight-lever render
// Based on CoreRender (GLCoreRender or DX11CoreRender)
//
class Render : public IRender, IProfilerCallback
{
	ICoreRender *_pCoreRender{ nullptr };
	IResourceManager *_pResMan{ nullptr };
	ISceneManager *_pSceneMan{ nullptr };
	IFileSystem *_fsystem{ nullptr };

	WRL::ComPtr<ITextFile> _forwardShader;
	WRL::ComPtr<ITextFile> _postShader;
	WRL::ComPtr<ITextFile> _idShader;
	WRL::ComPtr<ITextFile> _fontShader;

	MeshPtr _postPlane;
	RenderTargetPtr renderTarget;
	TexturePtr whiteTexture;

	TexturePtr fontTexture;

	struct TexturePoolable
	{
		int64_t frame;
		int free;
		uint width;
		uint height;
		TEXTURE_FORMAT format;
		TexturePtr tex;
	};
	vector<TexturePoolable> _texture_pool;

	std::unordered_map<ShaderRequirement, ShaderPtr, ShaderRequirement> _shaders_pool;

	struct RenderMesh
	{
		uint model_id;
		IMesh *mesh{ nullptr };
		mat4 modelMat;
	};

	// Frame data
	mat4 ViewMat;
	mat4 ViewProjMat;

	// One Profiler character
	struct charr
	{
		float data[4];
		uint32_t id;
		uint32_t __align[3];
	};

	// One profiler line
	struct RenderProfileRecord
	{
		size_t txtHash{};
		size_t length{};
		unique_ptr<charr[]> bufferData;
		StructuredBufferPtr buffer;
	};

	vector<RenderProfileRecord> _records;

	API shaders_reload(const char **args, uint argsNumber);

	uint getNumLines() override;
	string getString(uint i) override;

	void renderForward(RenderBuffers& buffers, vector<RenderMesh>& meshes);
	void renderEnginePost(RenderBuffers& buffers);
	void setShaderMeshParameters(RENDER_PASS pass, RenderMesh *mesh, IShader *shader);
	void drawMeshes(vector<RenderMesh>& meshes, RENDER_PASS pass);
	void _update();
	IShader* getShader(const ShaderRequirement &req);
	bool isOpenGL();
	void getRenderMeshes(vector<RenderMesh>& meshes);	
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
	ITexture *WhiteTexture() { return whiteTexture.Get(); }

	API PreprocessStandardShader(OUT IShader **pShader, const ShaderRequirement *shaderReq) override;
	API RenderPassIDPass(const ICamera *pCamera, ITexture *tex, ITexture *depthTex) override;
	API RenderPassGUI() override;
	API GetRenderTexture2D(OUT ITexture **texOut, uint width, uint height, TEXTURE_FORMAT format) override;
	API ReleaseRenderTexture2D(ITexture *texIn) override;
	API ShadersReload() override;
	API GetName(OUT const char **pName) override;
};

