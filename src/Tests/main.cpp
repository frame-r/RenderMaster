#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::OPENGL45 | INIT_FLAGS::MSAA_8X, L"resources", nullptr)))
		{
			IResourceManager *resMan;
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			IModel *pModel;
			resMan->LoadModel(&pModel, "box.fbx");
			pModel->AddRef();

			IModel *m2;
			resMan->LoadModel(&m2, "box.fbx");
			m2->AddRef();

			vec3 p = {11.0f, 0.0f, 0.0f};
			m2->SetPosition(&p);

			pCore->Start(); // Begin main loop

			pModel->Release();
			m2->Release();

			pCore->ReleaseEngine();
		}

		FreeCore(pCore);
	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}



