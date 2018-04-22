#pragma once
#include "Common.h"


enum class INPUT_ATTRUBUTE
{
	POSITION,
	NORMAL,
	TEX_COORD
};

class Render
{
	ICoreRender *_pCoreRender{nullptr};
	ISceneManager *_pSceneMan{nullptr};
	IResourceManager *_pResMan{nullptr};
	IFileSystem *_fsystem{nullptr};
	ShaderText pStandardShaderText;

	void save_text(std::list<std::string>& l, const std::string&& str);
	ICoreShader* _get_shader(INPUT_ATTRUBUTE attributes);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void RenderFrame();
};

