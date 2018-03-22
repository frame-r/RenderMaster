#include "Engine.h"

using namespace RENDER_MASTER;


ICore* pCore;

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{	
	if (GetCore(pCore))
	{
		pCore->Init(INIT_FLAGS::IF_SELF_WINDOW | INIT_FLAGS::IF_CONSOLE, nullptr, "C:\\Users\\Konstantin\\Documents\\RenderMasterProject1");
		
		IResourceManager *resMan;
		pCore->GetSubSystem((ISubSystem*&)resMan, SUBSYSTEM_TYPE::ST_RESOURCE_MANAGER);

		IModel *pmdl1, *pmdl2;
		resMan->CreateDefaultModel(pmdl1, DEFAULT_RESOURCE_TYPE::DRT_PLANE);
		resMan->CreateDefaultModel(pmdl2, DEFAULT_RESOURCE_TYPE::DRT_PLANE);

		IModel *pModel;
		resMan->LoadModel(pModel, "box.fbx", nullptr);

		pCore->CloseEngine();

		FreeCore(pCore);
	}

	return 0;
}



