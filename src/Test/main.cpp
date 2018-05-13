#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		IResourceManager *resMan;
		IModel *pModel;
		ISceneManager *sm;

		pCore->Init(INIT_FLAGS::CREATE_CONSOLE, "resources", nullptr);
				
		pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
		pCore->GetSubSystem((ISubSystem**)&sm, SUBSYSTEM_TYPE::SCENE_MANAGER);
			
		resMan->LoadModel(&pModel, "sponza.fbx", nullptr);

		mat4 m(1.0f);
		m.el_2D[2][3] = 1.0f;
		vec4 v(0.0f, 0.0f, 1.0f, 1.0f);
		vec4 v1 = m * v;
				
		pCore->Start(); // begin main loop

		pCore->CloseEngine();

		FreeCore(pCore);

	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}



