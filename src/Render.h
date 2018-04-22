#pragma once
#include "Common.h"

enum class INPUT_ATTRUBUTE
{
	POSITION = 1 << 0,
	NORMAL = 1 << 1,
	TEX_COORD = 1 << 2
};
DEFINE_ENUM_OPERATORS(INPUT_ATTRUBUTE)


class Render
{

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

	ICoreRender *_pCoreRender{nullptr};
	ISceneManager *_pSceneMan{nullptr};
	IResourceManager *_pResMan{nullptr};
	IFileSystem *_fsystem{nullptr};

	ShaderText pStandardShaderText;

	std::unordered_map<ShaderRequirement, ICoreShader*, ShaderRequirement> _shader_pool;

	void save_text(std::list<std::string>& l, const std::string&& str);
	ICoreShader* _get_shader(const ShaderRequirement &req);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void RenderFrame();
};

