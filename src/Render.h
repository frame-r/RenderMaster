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

	ShaderText pStandardShaderText;

	ICoreShader* _get_shader(INPUT_ATTRUBUTE attributes, bool texture);

public:

	Render(ICoreRender *pCoreRender);
	~Render();

	void RenderFrame();
};

