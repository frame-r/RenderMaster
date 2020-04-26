#include "pch.h"
#include "gameobject.h"
#include "camera.h"
#include "model.h"
#include "light.h"
#include "resource_manager.h"
#include "filesystem.h"
#include <assert.h>
#include "core.h"
#include "console.h"
#include "yaml-cpp/yaml.h"
#include "fbx.h"
#include "images.h"

#define IMPORT_DIR ".import"
#define UNLOAD_RESOURCE_FRAMES 10

using namespace YAML;

class MeshResource;
class TextureResource;

// Runtime
static std::set<Texture*> texturesSet;
static std::set<Shader*> shadersSet;
static std::set<StructuredBuffer*> structuredBuffersSet;

// Managed (From file)
static std::unordered_map<string, TextureResource*> streamTexturesMap;
static std::unordered_map<string, MeshResource*> streamMeshesMap;

// Root GameObjects
static std::vector<GameObject*> rootObjectsVec;


SharedPtr<Texture> ResourceManager::CreateTexture(int width, int height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags)
{
	ICoreTexture *coreTex = CORE_RENDER->CreateTexture(nullptr, width, height, type, format, flags, false);

	Texture *texture = new Texture(unique_ptr<ICoreTexture>(coreTex));
	texturesSet.emplace(texture);

	auto removeTex = [](Texture* t)
	{
		texturesSet.erase(t);
		delete t;
	};

	return SharedPtr<Texture>(texture, removeTex);
}

auto DLLEXPORT ResourceManager::CreateShader(const char *vert, const char *geom, const char *frag) -> SharedPtr<Shader>
{
	auto removeShader = [](Shader* s)
	{
		shadersSet.erase(s);
		delete s;
	};

	ERROR_COMPILE_SHADER err;
	ICoreShader* coreShader = CORE_RENDER->CreateShader(vert, frag, geom, err);

	if (!coreShader)
	{
		const char *shaderText = nullptr;

		string err_compile = "err_compile";

		switch (err)
		{
			case ERROR_COMPILE_SHADER::VERTEX: shaderText = vert; err_compile += "_vertex"; break;
			case ERROR_COMPILE_SHADER::FRAGMENT: shaderText = frag; err_compile += "_fragment"; break;
			case ERROR_COMPILE_SHADER::GEOM: shaderText = geom; err_compile += "_geom"; break;
		};

		err_compile += ".shader";
	
		File f = FS->OpenFile(err_compile.c_str(), FILE_OPEN_MODE::WRITE);
		f.Write((uint8 *)shaderText, strlen(shaderText));

		return SharedPtr<Shader>(nullptr, removeShader);
	}

	Shader *sh = new Shader(unique_ptr<ICoreShader>(coreShader), unique_ptr<const char[]>(vert), unique_ptr<const char[]>(geom), unique_ptr<const char[]>(frag));
	shadersSet.emplace(sh);
	return SharedPtr<Shader>(sh, removeShader);
}

auto DLLEXPORT ResourceManager::CreateComputeShader(const char* compText) -> SharedPtr<Shader>
{
	auto removeShader = [](Shader* s)
	{
		shadersSet.erase(s);
		delete s;
	};

	ERROR_COMPILE_SHADER err;
	ICoreShader* coreShader = CORE_RENDER->CreateComputeShader(compText, err);

	if (!coreShader)
	{
		const char* shaderText = nullptr;

		switch (err)
		{
			case ERROR_COMPILE_SHADER::COMP: shaderText = compText; break;
			default: abort(); // unreal
		};

		File f = FS->OpenFile("err_compile.shader", FILE_OPEN_MODE::WRITE);
		f.Write((uint8*)shaderText, strlen(shaderText));

		return SharedPtr<Shader>(nullptr, removeShader);
	}

	Shader* sh = new Shader(unique_ptr<ICoreShader>(coreShader), unique_ptr<const char[]>(compText));
	shadersSet.emplace(sh);
	return SharedPtr<Shader>(sh, removeShader);

}

auto DLLEXPORT ResourceManager::CreateStructuredBuffer(uint size, uint elementSize, BUFFER_USAGE usage) -> SharedPtr<StructuredBuffer>
{
	ICoreStructuredBuffer *c = CORE_RENDER->CreateStructuredBuffer(size, elementSize, usage);

	StructuredBuffer *buffer = new StructuredBuffer(unique_ptr<ICoreStructuredBuffer>(c));
	structuredBuffersSet.emplace(buffer);

	auto removeBuffer = [](StructuredBuffer* b)
	{
		structuredBuffersSet.erase(b);
		delete b;
	};

	return SharedPtr<StructuredBuffer>(buffer, removeBuffer);
}

GameObject* ResourceManager::CreateGameObject()
{
	GameObject *g  = new GameObject;
	rootObjectsVec.push_back(g);
	onObjectAdded.Invoke(g);
	return g;
}

auto DLLEXPORT ResourceManager::CreateCamera() -> Camera *
{
	Camera *c = new Camera;
	rootObjectsVec.push_back(c);
	onObjectAdded.Invoke(c);
	return c;
}

auto DLLEXPORT ResourceManager::CreateModel(const char *path) -> Model*
{
	StreamPtr<Mesh> meshPtr = CreateStreamMesh(path);

	Model *m = new Model(meshPtr);
	rootObjectsVec.push_back(m);
	onObjectAdded.Invoke(m);
	return m;
}

auto DLLEXPORT ResourceManager::CreateLight() -> Light*
{
	Light *l = new Light;
	rootObjectsVec.push_back(l);
	onObjectAdded.Invoke(l);
	return l;
}

auto DLLEXPORT ResourceManager::CloneObject(GameObject * obj) -> GameObject *
{
	GameObject *ret = obj->Clone();

	if (std::find(rootObjectsVec.begin(), rootObjectsVec.end(), obj) != rootObjectsVec.end())
		rootObjectsVec.emplace_back(ret);
	else
	{
		GameObject *p = obj->GetParent();
		p->InsertChild(ret);
	}

	onObjectAdded.Invoke(ret);

	return ret;
}

auto DLLEXPORT ResourceManager::RemoveObject(GameObject *obj) -> void
{
	auto it = std::find(rootObjectsVec.begin(), rootObjectsVec.end(), obj);
	if (it != rootObjectsVec.end())
	{
		GameObject *g = *it;
		rootObjectsVec.erase(it);
		g->SetWorldTransform(g->GetWorldTransform());
	}
}

auto DLLEXPORT ResourceManager::DestroyObject(GameObject * obj) -> void
{
	if (auto childs = obj->GetNumChilds())
	{
		for (int i = int(childs - 1); i >= 0; i--)
			DestroyObject(obj->GetChild(i));
	}

	if (obj->GetParent() == nullptr)
	{
		auto it = std::find(rootObjectsVec.begin(), rootObjectsVec.end(), obj);
		assert(it != rootObjectsVec.end());
		GameObject *g = *it;
		rootObjectsVec.erase(it);
	} else
		obj->GetParent()->RemoveChild(obj);

	onObjectDestroy.Invoke(obj);

	delete obj;
}

auto DLLEXPORT ResourceManager::InsertObject(int row, GameObject *obj) -> void
{
	assert(row <= rootObjectsVec.size());

	rootObjectsVec.insert(rootObjectsVec.begin() + row, obj);
	obj->SetWorldTransform(obj->GetWorldTransform());
}

auto ResourceManager::GetNumObjects() -> size_t
{
	return rootObjectsVec.size();
}

auto ResourceManager::GetObject_(size_t i) -> GameObject*
{
	return rootObjectsVec[i];
}

auto ResourceManager::GetImportMeshDir() -> std::string
{
	return _core->GetDataPath() + '\\' + IMPORT_DIR;
}

auto ResourceManager::GetImportTextureDir() -> std::string
{
	return _core->GetDataPath() + '\\' + IMPORT_DIR;
}

auto DLLEXPORT ResourceManager::AddCallbackOnObjAdded(ObjectCallback c) -> void
{
	onObjectAdded.Add(c);
}

bool findRecursive(GameObject *root, int id, GameObject *&found)
{
	GameObject* ret;

	if (root->GetId() == id)
	{
		found = root;
		return true;
	}

	for (int i = 0; i<root->GetNumChilds(); i++)
	{
		if (findRecursive(root->GetChild(i), id, ret))
		{
			found = ret;
			return true;
		}
	}

	found = nullptr;
	return false;
}

auto DLLEXPORT ResourceManager::FindObjectById(int id) -> GameObject*
{
	GameObject *ret;

	for (int i = 0; i < rootObjectsVec.size(); i++)
	{
		if (findRecursive(rootObjectsVec[i], id, ret))
			return ret;
	}

	return nullptr;
}

auto DLLEXPORT ResourceManager::RemoveCallbackOnObjAdded(ObjectCallback c) -> void
{
	onObjectAdded.Erase(c);
}

auto DLLEXPORT ResourceManager::AddCallbackOnDestroy(ObjectCallback c) -> void
{
	onObjectDestroy.Add(c);
}

auto DLLEXPORT ResourceManager::RemoveCallbackOnObjDestroyed(ObjectCallback c) -> void
{
	onObjectDestroy.Erase(c);
}

class TextureResource : public Resource<Texture>
{
protected:
	TEXTURE_CREATE_FLAGS flags_;
	Texture *create() override
	{
		return new Texture(path_, flags_);
	}

public:
	TextureResource(const std::string& path,TEXTURE_CREATE_FLAGS flags) : Resource(path),
		flags_(flags)
	{}
};

class MeshResource : public Resource<Mesh>
{
protected:
	TEXTURE_CREATE_FLAGS flags_;
	Mesh *create() override
	{
		return new Mesh(path_);
	}

public:
	MeshResource(const std::string& path) : Resource(path)
	{}
};

auto DLLEXPORT ResourceManager::CreateStreamTexture(const char *path, TEXTURE_CREATE_FLAGS flags) -> StreamPtr<Texture>
{
	auto it = streamTexturesMap.find(path);
	if (it != streamTexturesMap.end())
		return StreamPtr<Texture>(it->second);

	TextureResource *resource = new TextureResource(path, flags);
	streamTexturesMap[path] = resource;
	return StreamPtr<Texture>(resource);
}


auto DLLEXPORT ResourceManager::CreateStreamMesh(const char *path) -> StreamPtr<Mesh>
{
	auto it = streamMeshesMap.find(path);
	if (it != streamMeshesMap.end())
		return StreamPtr<Mesh>(it->second);

	MeshResource *resource = new MeshResource(path);
	streamMeshesMap[path] = resource;
	return StreamPtr<Mesh>(resource);
}

auto DLLEXPORT ResourceManager::GetImportedMeshes() -> std::vector<std::string>
{
	vector<string> paths = FS->FilterPaths(".mesh");

	std::vector<std::string> stds = { "std#plane", "std#grid", "std#line", "std#axes_arrows", "std#cube" };
	paths.insert(paths.begin(), stds.begin(), stds.end());

	return paths;
}

auto DLLEXPORT ResourceManager::Import(const char *path, ProgressCallback callback) -> void
{
	Log("Importing '%s'...", path);

	if (!FS->FileExist(path))
	{
		LogCritical("Importing failed: file %s not found", path);
		return;
	}

	const string ext = fileExtension(path);

	// TODO: copy file to resources

	if (ext == "fbx")
		importFbx(path, callback);
	else if (ext == "jpg" || ext == "jpeg")
		importJPEG(path);
	else if (ext == "png")
		importPNG(path);
	else
	{
		LogCritical("Importing failed: importer for extension %s not found", ext.c_str());
		return;
	}
}

class ResManProfiler : public IProfilerCallback
{
public:
	uint getNumLines() override { return 5; }
	std::string getString(uint i) override
	{
		size_t texBytes = 0;
		size_t textures = texturesSet.size();
		for(Texture *t : texturesSet)
			texBytes += t->GetVideoMemoryUsage();
		for(auto [key, resource] : streamTexturesMap)
		{
			if (resource->isLoaded())
			{
				texBytes += resource->getVideoMemoryUsage();
				textures++;
			}
		}

		size_t meshBytes = 0;
		size_t meshes = 0;
		for(auto [key, resource] : streamMeshesMap)
		{
			if (resource->isLoaded())
			{
				meshBytes += resource->getVideoMemoryUsage();
				meshes++;
			}
		}
		size_t buffersBytes = 0;
		for(StructuredBuffer *b : structuredBuffersSet)
			buffersBytes += b->GetVideoMemoryUsage();

		switch (i)
		{
			case 0: return "==== Resource Manager ====";
			case 1: return "Textures: " + std::to_string(textures) + " (" + bytesToMBytes(texBytes) + " Mb)";
			case 2: return "Meshes: " + std::to_string(meshes) + " (" + bytesToMBytes(meshBytes) + " Mb)";
			case 3: return "Structured Buffers: " + std::to_string(structuredBuffersSet.size()) + " (" + bytesToMBytes(buffersBytes) + " Mb)";
		}
		return "";
	}
} profiler;

void ResourceManager::Init()
{
	_core->AddProfilerCallback(&profiler);
	Log("ResourceManager Inited");
}
void ResourceManager::Free()
{
	_core->RemoveProfilerCallback(&profiler);

	CloseWorld();

	assert(shadersSet.size() == 0);
	assert(texturesSet.size() == 0);
	assert(structuredBuffersSet.size() == 0);

	Log("ResourceManager Free");
}
void ResourceManager::Update(float dt)
{
	for (GameObject *g : rootObjectsVec)
		g->Update(dt);

	for (auto [p, m] : streamMeshesMap)
	{
		if ((_core->frame() - m->frame() > UNLOAD_RESOURCE_FRAMES) && m->isLoaded())
			m->free();
	}
	for (auto [p, m] : streamTexturesMap)
	{
		if ((_core->frame() - m->frame() > UNLOAD_RESOURCE_FRAMES) && m->isLoaded())
			m->free();
	}
}

void ResourceManager::Reload()
{
	// TODO
	Log("ResourceManager: Reloading all resources...");
}

void saveObj(Emitter& out, GameObject *o)
{
	if (!o)
		return;

	out << YAML::BeginMap;
	out << Key << "childs" << Value << o->GetNumChilds();
	o->SaveYAML(static_cast<void*>(&out));
	out << YAML::EndMap;

	for (int i = 0; i < o->GetNumChilds(); i++)
		saveObj(out, o->GetChild(i));
}

auto DLLEXPORT ResourceManager::SaveWorld() -> void
{
	Log("Saving %s ...", "scene.yaml");

	YAML::Emitter out;

	out << YAML::BeginMap;

	out << Key << "roots" << Value << rootObjectsVec.size();

	out << Key << "objects" << Value;
	out << YAML::BeginSeq;

	for (int i = 0; i< rootObjectsVec.size(); i++)
		saveObj(out, rootObjectsVec[i]);

	out << YAML::EndSeq;
	out << YAML::EndMap;

	File f = FS->OpenFile("scene.yaml", FILE_OPEN_MODE::WRITE | FILE_OPEN_MODE::BINARY);
	
	f.WriteStr(out.c_str());
}

void loadObj(YAML::Node& objects_yaml, int *i, GameObject *parent, Signal<GameObject*> &sig)
{
	if (*i >= objects_yaml.size())
		return;

	auto obj_yaml = objects_yaml[*i];
	*i = *i + 1;

	int childs = obj_yaml["childs"].as<int>();
	OBJECT_TYPE type = getTypeByName(obj_yaml["type"].as<std::string>());

	GameObject *g = nullptr;

	switch (type)
	{
		case OBJECT_TYPE::GAMEOBJECT: g = new GameObject; break;
		case OBJECT_TYPE::MODEL:
		{
			string mesh = obj_yaml["mesh"].as<string>();
			StreamPtr<Mesh> meshPtr = RES_MAN->CreateStreamMesh(mesh.c_str());
			g = new Model(meshPtr);
		}
		break;
		case OBJECT_TYPE::LIGHT: g = new Light; break;
		case OBJECT_TYPE::CAMERA: g = new Camera; break;
		default:
			assert("unknown type");
			break;
	}	

	if (parent)
		parent->InsertChild(g);
	else
		rootObjectsVec.push_back(g);

	g->LoadYAML(static_cast<void*>(&obj_yaml));

	sig.Invoke(g);

	for (int j = 0; j < childs; j++)
		loadObj(objects_yaml, i, g, sig);
}

auto DLLEXPORT ResourceManager::LoadWorld() -> void
{
	if (rootObjectsVec.size())
	{
		LogWarning("ResourceManager::LoadWorld(): scene loaded");
		return;
	}

	File f = FS->OpenFile("scene.yaml", FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	
	uint fileSize =	(uint)f.FileSize();
	
	unique_ptr<char[]> tmp = unique_ptr<char[]>(new char[fileSize + 1L]);
	tmp[fileSize] = '\0';
	
	f.Read((uint8 *)tmp.get(), fileSize);
	
	YAML::Node model_yaml = YAML::Load(tmp.get());
	auto t = model_yaml.Type();

	if (!model_yaml["roots"])
	{
		LogCritical("LoadWorld(): invalid file");
		return;
	}

	auto roots_yaml = model_yaml["roots"];

	if (roots_yaml.Type() != NodeType::Scalar)
	{
		LogCritical("LoadWorld(): invalid file");
		return;
	}

	auto roots = roots_yaml.as<int>();
	if (roots <= 0)
	{
		Log("LoadWorld(): world is empty");
		return;
	}

	auto objects_yaml = model_yaml["objects"];
	if (objects_yaml.Type() != NodeType::Sequence)
	{
		LogCritical("LoadWorld(): invalid file");
		return;
	}

	int i = 0;
	for (int j = 0; j < roots; j++)
		loadObj(objects_yaml, &i, nullptr, onObjectAdded);
}

auto DLLEXPORT ResourceManager::CloseWorld() -> void
{
	if (const int roots = (int)rootObjectsVec.size())
	{
		for (int i = roots - 1; i >= 0; i--)
			DestroyObject(rootObjectsVec[i]);
	}
}

