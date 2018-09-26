# RenderMaster

Graphic engine based on COM (Component Object Model). 

## Features
* [Editor](https://github.com/fra-zz-mer/RenderMasterEditor)
* Multi render: DirectX 11, OpenGL 4.5
* Stable ABI. i.e engine binaries stable across different compiler (that supports COM) and different release of one compiler. Also no need recompiler all engine plugins and applications that uses engine if you migrate to new compiler.

Planned:
* Deferred Rendering
* Physically Based Shading
* TAA or MSAA
* Voxel Cone Tracing
* SSAO, SSR

## Build and Install
Clone repositiory. Build solution Engine.sln located in build/ directory. I use Visual Studio 2017 (15.6.6), but I'm sure that the engine is compiled with VS 2015. Then you should register Engine.dll in Windows Registry. Just run install.bat as Administrator. Now everything is ready to work with the engine. See example build/Test.vcxproj for more details.

## Uninstall
To clean Windows Registry run uninstall.bat located ib build/ directory.

## FBX SDK
By default Engine solution configured to find FBX SDK at C:\Program Files\Autodesk\FBX\FBX SDK\2018.0\. To build engine without FBX SDK comment '#define USE_FBX' in include\Engine.h and remove libfbxsdk-md.lib from Engine project settings (Linker -> Input -> Additional Dependencies)

## Example
```cpp
#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		IResourceManager *resMan;

		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::DIRECTX11, "resources", nullptr)))
		{
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			ResourcePtr<IModel> pModel = resMan->loadModel("box.fbx");

			pCore->Start(); // begin main loop

			pModel.reset();

			pCore->CloseEngine();
		}

		FreeCore(pCore);
	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}




```
![Alt text](box.png?raw=true "Test")

