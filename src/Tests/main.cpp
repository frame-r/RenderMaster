#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		IResourceManager *resMan;

		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::OPENGL45 | INIT_FLAGS::MSAA_32X, "resources", nullptr)))
		{
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			IResource*i;
			resMan->LoadMesh(&i, "hhhh\\path:mesh");

			ResourcePtr<IModel> pModel = resMan->loadModel("box.fbx");
			ResourcePtr<IModel> pModel1 = resMan->loadModel("box.fbx");
			pModel1->SetPosition(&vec3(11.0f, 0.0, 0.0));

			pCore->Start(); // begin main loop

			pModel.reset(); // reset this pointer in order to engine can release this resource
			pModel1.reset(); // reset this pointer in order to engine can release this resource

			pCore->ReleaseEngine();
		}

		FreeCore(pCore);
	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}



