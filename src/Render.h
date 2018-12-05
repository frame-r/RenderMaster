#pragma once
#include "Common.h"

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
	WRL::ComPtr<IShaderFile> _idShader;

	WRL::ComPtr<ITexture> _idTex;
	WRL::ComPtr<IRenderTarget> _idTexRT;

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

	void _update();

	IShader* _get_shader(const ShaderRequirement &req);

	bool _is_opengl();

	void _get_render_mesh_vec(vector<RenderMesh>& meshes);
	void _draw_meshes(const mat4& VP, vector<RenderMesh>& meshes, RENDER_PASS pass);

	ITexture* _get_render_target_texture_2d(uint width, uint height, TEXTURE_FORMAT format);
	void _release_texture_2d(ITexture *tex);

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

