#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		IResourceManager *resMan;

		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::DIRECTX11 | INIT_FLAGS::MSAA_8X, L"resources", nullptr)))
		{
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			{
				IModel *pModel;
				resMan->LoadModel(&pModel, "box.fbx");
				pModel->AddRef();

				//ITexture *tex;
				//resMan->CreateTexture(&tex, 100, 100, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS::FILTER_POINT | TEXTURE_CREATE_FLAGS::USAGE_RENDER_TARGET);

				pCore->Start(); // Begin main loop

				//tex->Release();
				pModel->Release();
			} 

			pCore->ReleaseEngine();
		}

		FreeCore(pCore);
	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}



