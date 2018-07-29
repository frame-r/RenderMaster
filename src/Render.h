#pragma once
#include "Common.h"

class Render : public IRender
{
	ICoreRender *_pCoreRender{nullptr};
	IResourceManager *_pResMan{nullptr};
	ISceneManager *_pSceneMan{nullptr};
	IFileSystem *_fsystem{nullptr};

	ShaderText pStandardShaderText;

	float _aspect{1.0f};	

	std::unordered_map<ShaderRequirement, ICoreShader*, ShaderRequirement> _shaders_pool;
	
	struct TRenderMesh
	{
		ICoreMesh *mesh{nullptr};
		mat4 modelMat;
	};

	ICoreShader* _get_shader(const ShaderRequirement &req);
	void _export_shader_to_file(std::list<std::string>& text, const std::string&& file);	
	void _create_render_mesh_vec(std::vector<TRenderMesh>& meshes);
	void _sort_meshes(std::vector<TRenderMesh>& meshes);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void Init();
	void RenderFrame(const ICamera *pCamera);

	API GetShader(OUT ICoreShader **pShader, const ShaderRequirement *shaderReq) override;
	API GetName(OUT const char **pName) override;
};

