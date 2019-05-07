# RenderMaster

Graphic engine.

## Features
* Deferred render
* Physically based materials
* Image based lighting
* Game objects system with parent-child relationship
* [Editor](https://github.com/fra-zz-mer/RenderMasterEditor)

## Building
Open solution build/Engine.sln. Build yaml-cpp project first. Then build Engine project. Then the project Example can start the engine.
Note: Solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0

## Example
```cpp
#include "core.h"
#include "resource_manager.h"
#include "gameobject.h"
#include "camera.h"
#include "model.h"
#include "filesystem.h"
#include "console.h"

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	Core *core = GetCore();
	if (!core)
		return 0;

	core->Init("", nullptr, INIT_FLAGS::VSYNC_ON);

	ResourceManager *resMan = core->GetResourceManager();
	resMan->LoadWorld();

	Camera *cam = resMan->CreateCamera();

	core->Start(cam);

	core->Free();
	ReleaseCore(core);

	return 0;
}
```
![Alt text](title.png?raw=true "Example")



