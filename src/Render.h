#pragma once
#include "Common.h"

class Render
{
	ICoreRender *_pCoreRender{nullptr};
	ISceneManager *_pSceneMan{nullptr};
	IResourceManager *_pResMan{nullptr};
	IFileSystem *_fsystem{nullptr};
	ShaderText pStandardShaderText;

	struct ShaderRequirement
	{
		INPUT_ATTRUBUTE attributes;
		bool alphaTest;

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

	void save_text(std::list<std::string>& l, const std::string&& str);
	ICoreShader* _get_shader(const ShaderRequirement &req);
	void _get_meshes(std::vector<ICoreMesh*>& meshes);
	void _sort_meshes(std::vector<ICoreMesh*>& meshes);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void RenderFrame();
};
