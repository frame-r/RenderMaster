#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::OPENGL45, L"resources", nullptr)))
		{
			IResourceManager *resMan;
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			{
				IModel* m;

				resMan->LoadModel(&m, "box.fbx");
				ModelPtr model1(m);

				resMan->LoadModel(&m, "box.fbx");
				ModelPtr model2(m);
				model2->SetPosition(&vec3{ 11.0f, 0.0f, 0.0f });

				pCore->Start(); // Begin main loop
			}

			pCore->ReleaseEngine();
		}

		FreeCore(pCore);
	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}



