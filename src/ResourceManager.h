#pragma once
#include "Common.h"

#ifdef USE_FBX
#include <fbxsdk.h>
#endif

template<typename T>
class TResource : public IResource
{
	// Pointer to object
	// Every type which supports Free() method
	//
	T *_pointer = nullptr;

	// Type of pointer to which you can safely cast
	//
	RES_TYPE _type;

	// Unique string identificator
	// Example:
	// - box.fbx
	// - box.fbx:mesh01
	// - picture.png
	//
	string _resourceID;

	uint _refCount = 0;

private:

	void _free()
	{
		if (_pointer == nullptr)
			return;

		if (_refCount != 0)
		{
			LOG_WARNING_FORMATTED("TResource::Free(): unable delete resource because refs = %i!\n", _refCount);
			return;
		}

		IResourceManager *_pResMan;
		_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
		_pResMan->DeleteResource(this);

		LOG_WARNING_FORMATTED("Deleting resource %s", _resourceID.c_str());

		_pointer->Free();

		delete _pointer;
		_pointer = nullptr;
	}

public:

	TResource(T* pointerIn, RES_TYPE type, const string& file) : _pointer(pointerIn), _type(type), _resourceID(file) {}
	virtual ~TResource() { _free(); }

	T *get() { return _pointer; }
	inline T *operator->() { return _pointer; }

	API AddRef() override { _refCount++; return S_OK; }
	API Release() override
	{
		if (_refCount == 0)
		{
			_free();
			delete this;
			return S_OK;
		}

		_refCount--;

		if (_refCount > 0)
			return S_OK;

		_free();
		delete this;
		return S_OK;
	}
	API RefCount(OUT uint *refs) override { *refs = _refCount; return S_OK; }
	API GetType(OUT RES_TYPE *typeOut) override { *typeOut = _type; return S_OK; }
	API GetFileID(OUT const char **id) override { *id = _resourceID.c_str(); return S_OK; }
	API GetPointer(OUT void **pointerOut) override { *pointerOut = dynamic_cast<T*>(_pointer); return S_OK; }
};


class ResourceManager final : public IResourceManager
{
	std::unordered_set<IResource*> _resources;
	std::unordered_set<IResource*> _cache_resources;
		
	ICoreRender *_pCoreRender{nullptr};
	IFileSystem *_pFilesystem{nullptr};

	CRITICAL_SECTION _cs{};

	#ifdef USE_FBX
	const int fbxDebug = 0;

	void _InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void _DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

	vector<IResource*> _FBXLoadMeshes(const char *pFullPath, const char *pRelativePath);
	bool _LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void _LoadSceneHierarchy(vector<IResource*>& meshes, FbxScene* pScene, const char *pFullPath, const char *pRelativePath);
	void _LoadNode(vector<IResource*>& meshes, FbxNode* pNode, int pDepth, const char *pFullPath, const char *pRelativePath);
	void _LoadMesh(vector<IResource*>& meshes, FbxMesh *pMesh, FbxNode *pNode, const char *pFullPath, const char *pRelativePath);
	void _LoadNodeTransform(FbxNode* pNode, const char *str);
	#endif

	const char* _resourceToStr(IResource *pRes);
	IResource *_createResource(void *pointer, RES_TYPE type, const string& file);
	API _resources_list(const char **args, uint argsNumber);
	string constructFullPath(const string& file);
	bool check_file_not_exist(const string& fullPath);
	void collect_model_mesh(vector<IResource*>& res_out, std::unordered_set<IResource*> res_vec, const char* pRelativeModelPath, const char *pMeshID);

public:

	ResourceManager();
	virtual ~ResourceManager();

	void Init();
	
	API LoadModel(OUT IResource **pModelResource, const char *pModelPath) override;
	API LoadMesh(OUT IResource **pModelResource, const char *pMeshPath) override;
	API LoadShaderText(OUT IResource **pShaderResource, const char *pVertName, const char *pGeomName, const char *pFragName) override;
	API LoadTexture(OUT IResource **pTextureResource, const char *pMeshPath, TEXTURE_CREATE_FLAGS flags) override;
	//API CreateTexture(OUT ICoreTexture **pTextureResource, uint width, uint height, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags) override;
	API CreateUniformBuffer(OUT IResource **pResource, uint size) override;
	API CreateGameObject(OUT IResource **pResource, RES_TYPE type) override;
	API CloneGameObject(IResource *resourceIn, OUT IResource **resourceOut) override;
	API DeleteResource(IResource *pResource) override;
	API GetNumberOfResources(OUT uint *number) override;
	API GetName(OUT const char **pTxt) override;
	API Free() override;
};

