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
Clone repositiory. Build solution Engine.sln located in build/ directory. I use Visual Studio 2017 (15.8.2). Then you should register Engine.dll in Windows Registry. Just run cmd.exe as Administrator, navigate to build/ directory and type "install.bat release" for release build or "install.bat debug" for debug build. See build/Example.vcxproj for more details.

## Example
```cpp
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
```
![Alt text](box.png?raw=true "Test")

## Uninstall
To clean Windows Registry run uninstall.bat located ib build/ directory.

## FBX SDK
By default Engine solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0\. To build engine without FBX SDK comment '#define USE_FBX' in include\Engine.h and remove libfbxsdk-md.lib from Engine project settings (Linker -> Input -> Additional Dependencies)

## Resource management
We follow DirectX-style resource management. After loading resource through the ResourceManager you should call AddRef(). When you don't need resource you should call Release().

