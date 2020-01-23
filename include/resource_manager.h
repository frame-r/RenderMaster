#pragma once
#include "common.h"
#include "texture.h"
#include "mesh.h"
#include "shader.h"
#include "structured_buffer.h"


template<typename T>
class Resource;

template<typename T>
class ManagedPtr;

template<typename T>
using SharedPtr = std::shared_ptr<T>;

class ResourceManager
{
	template<typename T>
	friend class ManagedPtr;

	Signal<GameObject*> onObjectAdded;
	Signal<GameObject*> onObjectDestroy;

public:
	// Internal API
	void Reload();
	void Init();
	void Free();
	void Update(float dt);
	auto GetNumObjects() -> size_t;
	auto GetObject_(size_t i) -> GameObject*;
	auto GetImportMeshDir() -> std::string;
	auto GetImportTextureDir() -> std::string;

public:
	// Render runtime resources
	auto DLLEXPORT CreateTexture(int width, int height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags) -> SharedPtr<Texture>;
	auto DLLEXPORT CreateShader(const char *vert, const char *geom, const char *frag) -> SharedPtr<Shader>;
	auto DLLEXPORT CreateComputeShader(const char *compText) -> SharedPtr<Shader>;
	auto DLLEXPORT CreateStructuredBuffer(uint size, uint elementSize) -> SharedPtr<StructuredBuffer>;

	// Render stream resources
	auto DLLEXPORT CreateStreamTexture(const char *path, TEXTURE_CREATE_FLAGS flags) -> ManagedPtr<Texture>;
	auto DLLEXPORT CreateStreamMesh(const char *path) -> ManagedPtr<Mesh>;

	auto DLLEXPORT Import(const char *path, ProgressCallback callback) -> void;
	auto DLLEXPORT GetImportedMeshes() -> std::vector<std::string>;

	// TODO: Move all GameObject stuff to SceneManager class
	// Game objects
	auto DLLEXPORT CreateGameObject() -> GameObject*;
	auto DLLEXPORT CreateCamera() -> Camera*;
	auto DLLEXPORT CreateModel(const char *path) -> Model*;
	auto DLLEXPORT CreateLight()-> Light*;
	auto DLLEXPORT CloneObject(GameObject *obj)-> GameObject*;

	// Hierarchy
	auto DLLEXPORT RemoveObject(GameObject *obj) -> void;
	auto DLLEXPORT DestroyObject(GameObject *obj) -> void;
	auto DLLEXPORT InsertObject(int row, GameObject *obj) -> void;
	auto DLLEXPORT AddCallbackOnObjAdded(ObjectCallback c) -> void;
	auto DLLEXPORT FindObjectById(int id) -> GameObject*;
	auto DLLEXPORT RemoveCallbackOnObjAdded(ObjectCallback c) -> void;
	auto DLLEXPORT AddCallbackOnDestroy(ObjectCallback c) -> void;
	auto DLLEXPORT RemoveCallbackOnObjDestroyed(ObjectCallback c) -> void;

	auto DLLEXPORT SaveWorld() -> void;
	auto DLLEXPORT LoadWorld() -> void;
	auto DLLEXPORT CloseWorld() -> void;
};

