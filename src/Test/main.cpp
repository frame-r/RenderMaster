#include "Engine.h"

using namespace RENDER_MASTER;

ICore* pCore;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	
	if (GetEngine(pCore))
	{
		pCore->Init(INIT_FLAGS::IF_SELF_WINDOW | INIT_FLAGS::IF_CONSOLE, nullptr);		
		
		IResourceManager *resMan;
		pCore->GetSubSystem((ISubSystem*&)resMan, SUBSYSTEM_TYPE::ST_RESOURCE_MANAGER);

		IModel *pmdl1, *pmdl2;
		resMan->GetDefaultModel(pmdl1, DEFAULT_RESOURCE_TYPE::RT_PLANE);
		resMan->GetDefaultModel(pmdl2, DEFAULT_RESOURCE_TYPE::RT_PLANE);

		pCore->CloseEngine();
		FreeEngine(pCore);
	}

	return 0;
}



