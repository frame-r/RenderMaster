#include "core.h"
#include "resource_manager.h"
#include "gameobject.h"
#include "camera.h"
#include "model.h"
#include "filesystem.h"
#include "console.h"


#include "material_manager.h"
#include "material.h"

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	Core *core = GetCore();
	if (!core)
		return 0;

	core->Init("", nullptr, INIT_FLAGS::VSYNC_ON);

	ResourceManager *resMan = core->GetResourceManager();
	resMan->LoadWorld();

	Camera *c = resMan->CreateCamera();

	MaterialManager *mm = core->GetMaterialManager();

	Material *mat = mm->CreateMaterial("mesh");
	mat->SetTexture("albedo", "1.dds");
	//mat->SaveXML();

	core->Start(c);

	//resMan->RemoveObject(m);
	//delete m;
	//resMan->RemoveObject(c);
	//delete c;

	core->Free();
	ReleaseCore(core);


	//// textures
	//{
	//	SharedPtr<Texture> t = resMan->CreateTexture(1024, 1024);
	//	SharedPtr<Texture> t_ = t;

	//	ResourceSharedPtr<Texture> t1 = resMan->LoadTexture("1.dds");
	//	ResourceSharedPtr<Texture> t2 = resMan->LoadTexture("1.dds");
	//	ResourceSharedPtr<Texture> t3 = resMan->LoadTexture("2.dds");
	//}



	////core->ReloadCoreRender();

	//// gameobjects
	//{
	//	GameObject* g = resMan->CreateGameObject();

	//	g->SetWorldPosition({1,2,3});

	//	g->print_local();
	//	g->print_global();

	//	//SharedPtr<GameObject> g1 = g;
	//	//
	//	//SharedPtr<Light> l = resMan->CreateLight();
	//	//l->SetPosition(vec3(1,1,1));

	//	//SharedPtr<Model> m = resMan->CreateModel("");
	//	//SharedPtr<GameObject> gg = std::static_pointer_cast<GameObject>(m);
	//}

	//// fs
	//{
	//	FileSystem *fs = core->GetFilesystem();

	//	const char *p1 = "1.txt";
	//	core->GetConsole()->Log("file '%s' exist = %i", LOG_TYPE::NORMAL, p1, fs->FileExist(p1));

	//	const char *p2 = "C:\\Windows";
	//	core->GetConsole()->Log("path '%s' exist = %i", LOG_TYPE::NORMAL, p2, fs->DirectoryExist(p2));
	//}

	////  math
	//{
	//	vec3 t{32.f, 32.f, 444.f};
	//	quat r{10.0f, 70.0f, 10.0f};
	//	vec3 s{12.0f, 0.1f, 33.0f};

	//	mat4 m;
	//	compositeTransform(m, t, r, s);

	//	decompositeTransform(m, t, r, s);

	//	int y = 0;
	//}



	return 0;
}

