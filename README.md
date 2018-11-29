# RenderMaster

Graphic engine.

## Features
* [Editor](https://github.com/fra-zz-mer/RenderMasterEditor)
* Multi-render: DirectX 11, OpenGL 4.5
* MSAA
* Stable ABI (Application Binary Interface). I.e engine binaries stable across different compiler that supports COM and different release of one compiler. No need recompile engine if you migrate to new compiler.

Planned:
* Deferred Shading
* Physically Based Shading
* TAA
* SSAO, SSR
* Voxel Cone Tracing (or another solution for indirect light)

## Building and installation engine
Clone repositiory. Build solution Engine.sln located in build/ directory. I use Visual Studio 2017 (15.8.2), but I'm sure that the engine is compiled with VS 2015. Then you should register Engine.dll in Windows Registry. Just run cmd.exe as Administrator, navigate to build/ directory and type "install.bat release" for release build or "install.bat debug" for debug build. Now everything is ready to work with the engine. See example build/Test.vcxproj for more details.

## Example
```cpp
#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::DIRECTX11 | INIT_FLAGS::MSAA_8X, L"resources", nullptr)))
		{
			IResourceManager *resMan;
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			IModel *pModel;
			resMan->LoadModel(&pModel, "box.fbx");
			pModel->AddRef();

			pCore->Start(); // Begin main loop

			pModel->Release();

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

