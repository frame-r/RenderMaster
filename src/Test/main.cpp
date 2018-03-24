#include "Engine.h"

using namespace RENDER_MASTER;


ICore* pCore;

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{	
	if (GetCore(pCore))
	{
		IResourceManager *resMan;
		IModel *pModel, *pModel1, *pModel2;

		pCore->Init(INIT_FLAGS::CREATE_CONSOLE, "C:\\Users\\Konstantin\\Documents\\RenderMasterProject1", nullptr);
				
		pCore->GetSubSystem((ISubSystem*&)resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
			
		resMan->LoadModel(pModel, "humanoid.fbx", nullptr);
		resMan->GetDefaultModel(pModel1, DEFAULT_MODEL::PLANE);
		resMan->GetDefaultModel(pModel2, DEFAULT_MODEL::PLANE);

		pCore->CloseEngine();

		FreeCore(pCore);
	}

	return 0;
}



