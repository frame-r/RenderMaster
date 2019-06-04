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
#include "dds.h"

#define IMPORT_DIR ".import"
#define UNLOAD_RESOURCE_FRAMES 60

using namespace YAML;

class MeshResource;
class TextureResource;

// Runtime
static std::set<Texture*> textures_;
static std::set<Shader*> shaders_;
static std::set<StructuredBuffer*> structured_buffers_;

// Managed (From file)
static std::unordered_map<string, TextureResource*> resource_textures_;
static std::unordered_map<string, MeshResource*> resource_meshes_;

// Root GameObjects
static std::vector<GameObject*> root_objects_;


SharedPtr<Texture> ResourceManager::CreateTexture(int width, int height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags)
{
	ICoreTexture *coreTex = CORE_RENDER->CreateTexture(nullptr, width, height, type, format, flags, false);

	Texture *texture = new Texture(unique_ptr<ICoreTexture>(coreTex));
	textures_.emplace(texture);

	auto removeTex = [](Texture* t)
	{
		textures_.erase(t);
		delete t;
	};

	return SharedPtr<Texture>(texture, removeTex);
}

auto DLLEXPORT ResourceManager::CreateShader(const char *vert, const char *geom, const char *frag) -> SharedPtr<Shader>
{
	auto removeShader = [](Shader* s)
	{
		shaders_.erase(s);
		delete s;
	};

	ERROR_COMPILE_SHADER err;
	ICoreShader* coreShader = CORE_RENDER->CreateShader(vert, frag, geom, err);
	bool compiled = coreShader != nullptr;

	if (!compiled)
	{
		const char *shaderText = nullptr;

		switch (err)
		{
			case ERROR_COMPILE_SHADER::VERTEX: shaderText = vert; break;
			case ERROR_COMPILE_SHADER::FRAGMENT: shaderText = frag; break;
			case ERROR_COMPILE_SHADER::GEOM: shaderText = geom; break;
		};
	
		File f = FS->OpenFile("err_compile.shader", FILE_OPEN_MODE::WRITE);
		f.Write((uint8 *)shaderText, (uint)strlen(shaderText));

		return SharedPtr<Shader>(nullptr, removeShader);
	}

	Shader *sh = new Shader(unique_ptr<ICoreShader>(coreShader), unique_ptr<const char>(vert), unique_ptr<const char>(geom), unique_ptr<const char>(frag));
	shaders_.emplace(sh);
	return SharedPtr<Shader>(sh, removeShader);
}

auto DLLEXPORT ResourceManager::CreateStructuredBuffer(uint size, uint elementSize) -> SharedPtr<StructuredBuffer>
{
	ICoreStructuredBuffer *c = CORE_RENDER->CreateStructuredBuffer(size, elementSize);

	StructuredBuffer *buffer = new StructuredBuffer(unique_ptr<ICoreStructuredBuffer>(c));
	structured_buffers_.emplace(buffer);

	auto removeBuffer = [](StructuredBuffer* b)
	{
		structured_buffers_.erase(b);
		delete b;
	};

	return SharedPtr<StructuredBuffer>(buffer, removeBuffer);
}

GameObject* ResourceManager::CreateGameObject()
{
	GameObject *g  = new GameObject;
	root_objects_.push_back(g);
	onObjectAdded.Invoke(g);
	return g;
}

auto DLLEXPORT ResourceManager::CreateCamera() -> Camera *
{
	Camera *c = new Camera;
	root_objects_.push_back(c);
	onObjectAdded.Invoke(c);
	return c;
}

auto DLLEXPORT ResourceManager::CreateModel(const char *path) -> Model*
{
	ManagedPtr<Mesh> meshPtr = CreateStreamMesh(path);

	Model *m = new Model(meshPtr);
	root_objects_.push_back(m);
	onObjectAdded.Invoke(m);
	return m;
}

auto DLLEXPORT ResourceManager::CreateLight() -> Light*
{
	Light *l = new Light;
	root_objects_.push_back(l);
	onObjectAdded.Invoke(l);
	return l;
}

auto DLLEXPORT ResourceManager::CloneObject(GameObject * obj) -> GameObject *
{
	GameObject *ret = obj->Clone();

	auto it = std::find(root_objects_.begin(), root_objects_.end(), obj);
	if (it != root_objects_.end())
		root_objects_.emplace_back(ret);
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
	auto it = std::find(root_objects_.begin(), root_objects_.end(), obj);
	if (it != root_objects_.end())
	{
		GameObject *g = *it;
		root_objects_.erase(it);
		g->SetWorldTransform(g->GetWorldTransform());
	}
}

auto DLLEXPORT ResourceManager::DestroyObject(GameObject * obj) -> void
{
	int childs = (int)obj->GetNumChilds();
	if (childs > 0)
	{
		for (int i = childs - 1; i >= 0; i--)
		{
			DestroyObject(obj->GetChild(i));
		}
	}

	if (obj->GetParent() == nullptr)
	{
		auto it = std::find(root_objects_.begin(), root_objects_.end(), obj);
		assert(it != root_objects_.end());
		GameObject *g = *it;
		root_objects_.erase(it);
	} else
		obj->GetParent()->RemoveChild(obj);

	onObjectDestroy.Invoke(obj);

	delete obj;
}

auto DLLEXPORT ResourceManager::InsertObject(int row, GameObject *obj) -> void
{
	assert(row <= root_objects_.size());

	root_objects_.insert(root_objects_.begin() + row, obj);
	obj->SetWorldTransform(obj->GetWorldTransform());
}

auto ResourceManager::GetNumObjects() -> size_t
{
	return root_objects_.size();
}

auto ResourceManager::GetObject_(size_t i) -> GameObject*
{
	return root_objects_[i];
}

auto ResourceManager::GetImportMeshDir() -> std::string
{
	return _core->GetDataPath() + '\\' + IMPORT_DIR;
}

auto DLLEXPORT ResourceManager::AddCallbackOnObjAdded(ObjectCallback c) -> void
{
	onObjectAdded.Add(c);
}

bool findRecursive(GameObject *root, int id, GameObject *&found)
{
	if (root->GetId() == id)
	{
		found = root;
		return true;
	}

	GameObject *ret;
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
	for (int i = 0; i < root_objects_.size(); i++)
	{
		if (findRecursive(root_objects_[i], id, ret))
		{
			return ret;
		}
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

auto DLLEXPORT ResourceManager::CreateStreamTexture(const char *path, TEXTURE_CREATE_FLAGS flags) -> ManagedPtr<Texture>
{
	auto it = resource_textures_.find(path);
	if (it != resource_textures_.end())
	{
		return ManagedPtr<Texture>(it->second);
	}

	TextureResource *resource = new TextureResource(path, flags);
	resource_textures_[path] = resource;
	return ManagedPtr<Texture>(resource);
}


auto DLLEXPORT ResourceManager::CreateStreamMesh(const char *path) -> ManagedPtr<Mesh>
{
	auto it = resource_meshes_.find(path);
	if (it != resource_meshes_.end())
	{
		return ManagedPtr<Mesh>(it->second);
	}

	MeshResource *resource = new MeshResource(path);
	resource_meshes_[path] = resource;
	return ManagedPtr<Mesh>(resource);
}

auto DLLEXPORT ResourceManager::GetImportedMeshes() -> std::vector<std::string>
{
	std::vector<std::string> ret = {"std#plane", "std#grid", "std#line", "std#axes_arrows"};

	// TODO: add all imported mesh
	ret.push_back(".import\\teapot.mesh");

	return ret;
}

auto DLLEXPORT ResourceManager::Import(const char *path) -> void
{
	Log("Importing '%s'...", path);

	if (!FS->FileExist(path))
	{
		LogCritical("Importing failed: file %s not found", path);
		return;
	}

	importFbx(path);
}

class ResManProfiler : public IProfilerCallback
{
public:
	uint getNumLines() override { return 5; }
	std::string getString(uint i) override
	{
		size_t texBytes = 0;
		size_t textures = textures_.size();
		for(Texture *t : textures_)
			texBytes += t->GetVideoMemoryUsage();
		for(auto [key, resource] : resource_textures_)
		{
			if (resource->isLoaded())
			{
				texBytes += resource->getVideoMemoryUsage();
				textures++;
			}
		}

		size_t meshBytes = 0;
		size_t meshes = 0;
		for(auto [key, resource] : resource_meshes_)
		{
			if (resource->isLoaded())
			{
				meshBytes += resource->getVideoMemoryUsage();
				meshes++;
			}
		}
		size_t buffersBytes = 0;
		for(StructuredBuffer *b : structured_buffers_)
			buffersBytes += b->GetVideoMemoryUsage();

		switch (i)
		{
			case 0: return "==== Resource Manager ====";
			case 1: return "textures: " + std::to_string(textures) + " (" + bytesToMBytes(texBytes) + " Mb)";
			case 2: return "meshes: " + std::to_string(meshes) + " (" + bytesToMBytes(meshBytes) + " Mb)";
			case 3: return "structured buffers: " + std::to_string(structured_buffers_.size()) + " (" + bytesToMBytes(buffersBytes) + " Mb)";
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

	assert(shaders_.size() == 0);
	assert(textures_.size() == 0);
	assert(structured_buffers_.size() == 0);

	Log("ResourceManager Free");
}
void ResourceManager::Update(float dt)
{
	for (GameObject *g : root_objects_)
		g->Update(dt);

	for (auto [p, m] : resource_meshes_)
	{
		if ((_core->frame() - m->frame() > UNLOAD_RESOURCE_FRAMES) && m->isLoaded())
			m->free();
	}
	for (auto [p, m] : resource_textures_)
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
	if (!o) return;

	out << YAML::BeginMap;
	out << Key << "childs" << Value << o->GetNumChilds();
	o->Serialize(static_cast<void*>(&out));
	out << YAML::EndMap;

	for (int i = 0; i < o->GetNumChilds(); i++)
		saveObj(out, o->GetChild(i));
}

auto DLLEXPORT ResourceManager::SaveWorld() -> void
{
	Log("Saving %s ...", "scene.yaml");

	YAML::Emitter out;

	out << YAML::BeginMap;

	out << Key << "roots" << Value << root_objects_.size();

	out << Key << "objects" << Value;
	out << YAML::BeginSeq;

	for (int i = 0; i< root_objects_.size(); i++)
		saveObj(out, root_objects_[i]);

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
			ManagedPtr<Mesh> meshPtr = RES_MAN->CreateStreamMesh(mesh.c_str());
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
		root_objects_.push_back(g);

	g->Deserialize(static_cast<void*>(&obj_yaml));

	sig.Invoke(g);

	for (int j = 0; j < childs; j++)
		loadObj(objects_yaml, i, g, sig);
}

auto DLLEXPORT ResourceManager::LoadWorld() -> void
{
	if (root_objects_.size())
	{
		LogWarning("ResourceManager::LoadWorld(): scene loaded");
		return;
	}

	File f = FS->OpenFile("scene.yaml", FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	
	uint fileSize =	(uint)f.FileSize();
	
	unique_ptr<char[]> tmp = unique_ptr<char[]>(new char[fileSize + 1]);
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
	auto t1 = roots_yaml.Type();
	if (t1 != NodeType::Scalar)
	{
		LogCritical("LoadWorld(): invalid file");
		return;
	}
	
	int roots = roots_yaml.as<int>();
	if (roots <= 0)
	{
		Log("LoadWorld(): world is empty");
		return;
	}

	auto objects_yaml = model_yaml["objects"];
	auto t2 = objects_yaml.Type();
	if (t2 != NodeType::Sequence)
	{
		LogCritical("LoadWorld(): invalid file");
		return;
	}

	int i = 0;
	for (int j = 0; j < roots; j++)
	{
		loadObj(objects_yaml, &i, nullptr, onObjectAdded);
	}
}

auto DLLEXPORT ResourceManager::CloseWorld() -> void
{
	int roots = (int)root_objects_.size();
	if (roots)
	{
		for (int i = roots - 1; i >= 0; i--)
		{
			DestroyObject(root_objects_[i]);
		}
	}
}

