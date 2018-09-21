#pragma once
#include "Common.h"

#ifdef USE_FBX
#include <fbxsdk.h>
#endif

template<typename T>
class TResource : public IResource
{
	T *pointer = nullptr;
	uint refCount = 0;
	RES_TYPE type;

	void _free()
	{
		if (pointer == nullptr)
			return;

		if (refCount != 0)
		{
			LOG_WARNING_FORMATTED("TResource::Free(): unable delete resource because refs = %i!\n", refCount);
			return;
		}

		IResourceManager *_pResMan;
		_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
		_pResMan->DeleteResource(this);

		pointer->Free();
		delete pointer;
		pointer = nullptr;
	}

public:

	TResource(T* pointerIn) : pointer(pointerIn) {}
	virtual ~TResource() { _free(); }

	T *get() { return pointer; }

	inline T *operator->() { return pointer; }

	API AddRef() override { refCount++; return S_OK; }
	API Release() override
	{
		if (refCount == 0)
		{
			_free();
			delete this;
			return S_OK;
		}

		refCount--;

		if (refCount > 0)
			return S_OK;

		_free();
		delete this;

		return S_OK;
	}
	API RefCount(OUT uint *refs) { *refs = refCount; return S_OK; }
	API GetType(OUT RES_TYPE *typeOut) { *typeOut = type; return S_OK; }
	API GetPointer(OUT void **pointerOut) { *pointerOut = dynamic_cast<T*>(pointer); return S_OK; }
};


class ResourceManager final : public IResourceManager
{
	std::unordered_set<IResource*> _resources;
		
	ICoreRender *_pCoreRender{nullptr};
	IFileSystem *_pFilesystem{nullptr};

	CRITICAL_SECTION _cs{};

	#ifdef USE_FBX
	void _InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void _DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

	bool _FBXLoad(IModel *&pMesh, const char *pFileName);
	bool _LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void _LogSceneHierarchy(IModel *&pModel, FbxScene* pScene);
	void _LogNode(std::vector<TResource<ICoreMesh> *>& meshes, FbxNode* pNode, int pDepth);
	void _LogMesh(std::vector<TResource<ICoreMesh> *>& meshes, FbxMesh *pMesh, FbxNode *pNode);
	void _LogNodeTransform(FbxNode* pNode, const char *str);
	#endif

public:

	ResourceManager();
	~ResourceManager();

	void Init();
	
	API LoadModel(OUT IResource **pModel, const char *pFileName) override;
	API LoadShaderText(OUT IResource **pShader, const char *pVertName, const char *pGeomName, const char *pFragName) override;
	API CreateResource(OUT IResource **pResource, RES_TYPE type) override;
	API CreateUniformBuffer(OUT IResource **pResource, uint size) override;
	API DeleteResource(IResource *pResource) override;
	API GetNumberOfResources(OUT uint *number) override;
	API GetName(OUT const char **pTxt) override;
	API Free() override;
};

