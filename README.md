# RenderMaster

Graphic engine.

## Features
* DirectX 11, OpenGL 4.5.
* Stable ABI. I.e engine binaries are stable across different compiler that supports COM.
* [Editor](https://github.com/fra-zz-mer/RenderMasterEditor)

Planned:
* PBR
* Deferred or Forward+

## Building and installation engine
Clone repositiory. Build solution Engine.sln located in "build" directory (VS 2017 required). Then you should register Engine.dll in Windows Registry. Just run cmd.exe as Administrator, navigate to "build" directory and type "install.bat release" for release build or "install.bat debug" for debug build.

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
![Alt text](title.png?raw=true "Test")

## Uninstall
To clean Windows Registry run uninstall.bat located ib build/ directory.

## FBX SDK
By default Engine solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0\. To build engine without FBX SDK comment '#define USE_FBX' in include\Engine.h and remove libfbxsdk-md.lib from Engine project settings (Linker -> Input -> Additional Dependencies)

## Resource management
We follow DirectX-style resource management. After loading resource through the ResourceManager you should call AddRef(). When you don't need resource you should call Release().

