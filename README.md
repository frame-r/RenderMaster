# RenderMaster

Graphic engine based on COM (Component Object Model). 

## Install
Clone repositiory. Build solution Engine.sln located in build/ directory. Then you should register Engine.dll in Windows Registry. Just run install.bat as Administrator.
Now everything is ready to work with the engine. See example build/Test.vcxproj for more details.

## Uninstall
To clean Windows Registry run uninstall.bat located ib build/ directory.

## Example
```cpp
#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(pCore))
	{
		IResourceManager *resMan;
		IModel *pModel;

		pCore->Init(INIT_FLAGS::CREATE_CONSOLE, "..\\..\\..\\resources", nullptr);
				
		pCore->GetSubSystem((ISubSystem*&)resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
			
		resMan->LoadModel(pModel, "box.fbx", nullptr);
		
		pCore->CloseEngine();

		FreeCore(pCore);

	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}
```

