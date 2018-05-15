#pragma once
#include "Common.h"

class Render
{
	ICoreRender *_pCoreRender{nullptr};
	IResourceManager *_pResMan{nullptr};
	ISceneManager *_pSceneMan{nullptr};
	IFileSystem *_fsystem{nullptr};

	ShaderText pStandardShaderText;
	ICoreMesh *_pAxesMesh{ nullptr };

	float _aspect{1.0f};	

	struct ShaderRequirement
	{
		INPUT_ATTRUBUTE attributes{INPUT_ATTRUBUTE::CUSTOM};
		bool alphaTest{false};

		size_t operator()(const ShaderRequirement& k) const
		{
			return ((int)k.alphaTest + 1) * (int)k.attributes;
		}
		bool operator==(const ShaderRequirement &other) const
		{
			return attributes == other.attributes && alphaTest == other.alphaTest;
		}
	};
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
	void _draw_axes(const mat4& VP);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void Init();
	void RenderFrame(const ICamera *pCamera);
};

