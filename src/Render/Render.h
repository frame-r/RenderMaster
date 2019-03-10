#pragma once
#include "Common.h"

struct RenderBuffers
{
	uint height;
	uint width;

	// G-Buffer
	TexturePtr normal;		// RGB8		- World space normal
	TexturePtr shading;		// RGB8		- R - metallic, G - roughness
	TexturePtr depth;		// D24S8	- Hardware depth

	TexturePtr color;		// RGBA8	- Result tonemaped frame
	TexturePtr colorHDR;	// RGBA16F	- HDR frame
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

	std::unordered_map<ShaderRequirement, ShaderPtr, ShaderRequirement> _runtime_shaders;

	MeshPtr _quad;

	RenderTargetPtr renderTarget;

	TexturePtr whiteTexture;
	TexturePtr fontTexture;

	struct RenderTexture
	{
		int64_t frame;
		int free;
		uint width;
		uint height;
		TEXTURE_FORMAT format;
		TexturePtr tex;
	};
	vector<RenderTexture> _render_textures;

	struct RenderMesh
	{
		uint modelId;
		IMesh *mesh{nullptr};
		IMaterial *mat{nullptr};
		vec4 baseColor;
		vec4 shading;
		mat4 modelMat;
	};

	// Frame data
	mat4 ViewMat;
	mat4 ViewProjMat;
	vec4 cameraWorldPos;

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

	API_RESULT shaders_reload(const char **args, uint argsNumber);

	uint getNumLines() override;
	string getString(uint i) override;
	void renderForward(RenderBuffers& buffers, vector<RenderMesh>& meshes);
	void renderEnginePost(RenderBuffers& buffers);
	void setShaderMeshParameters(RENDER_PASS pass, RenderMesh *mesh, IShader *shader);
	void drawMeshes(vector<RenderMesh>& meshes, const char *shader, RENDER_PASS pass);
	void _update();
	IShader* getShader(const ShaderRequirement &req);
	vector<RenderMesh> getRenderMeshes();
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

	API_VOID PreprocessStandardShader(OUT IShader **pShader, const ShaderRequirement *shaderReq) override;
	API_VOID RenderPassIDPass(const ICamera *pCamera, ITexture *tex, ITexture *depthTex) override;
	API_VOID RenderPassGUI() override;
	API_VOID GetRenderTexture2D(OUT ITexture **texOut, uint width, uint height, TEXTURE_FORMAT format) override;
	API_VOID ReleaseRenderTexture2D(ITexture *texIn) override;
	API_VOID ShadersReload() override;
	API_RESULT GetName(OUT const char **pName) override;
};

