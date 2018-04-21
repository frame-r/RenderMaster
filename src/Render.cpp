#include "Render.h"
#include "Core.h"

extern Core *_pCore;

Render::Render(ICoreRender *pCoreRender) : _pCoreRender(pCoreRender)
{
	_pCore->GetSubSystem((ISubSystem*&)_pSceneMan, SUBSYSTEM_TYPE::SCENE_MANAGER);
	_pCore->GetSubSystem((ISubSystem*&)_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	_pResMan->LoadShaderText(pStandardShaderText, "mesh_vertex", nullptr, "mesh_fragment");
}


Render::~Render()
{
	//TODO!!
}

void Render::RenderFrame()
{
	_pCoreRender->Clear();
	_pCoreRender->SwapBuffers();
}
