# RenderMaster

Graphic engine based on COM (Component Object Model). 

## Installing
Clone and build solution Engine.sln located in build/ directory. Then you should register Engine.dll in Windows Registry. Just run install.reg.
Now everything is ready to work with the engine. See example build/Test.vcxproj for more details.

## Example
```cpp
#include "Engine.h"

using namespace RENDER_MASTER;

ICore* pCore;

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{	
	if (GetCore(pCore))
	{
		IResourceManager *resMan;
		IModel *pModel;

		pCore->Init(INIT_FLAGS::IF_SELF_WINDOW | INIT_FLAGS::IF_CONSOLE, nullptr, "<full path for all your assets>");
				
		pCore->GetSubSystem((ISubSystem*&)resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
			
		resMan->LoadModel(pModel, "<your FBX file>.fbx", nullptr);

		pCore->CloseEngine();

		FreeCore(pCore);
	}

	return 0;
}
```

