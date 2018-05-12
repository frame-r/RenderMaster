#pragma once
#include "Common.h"

#ifdef USE_FBX
#include <fbxsdk.h>
#endif


class ResourceManager final : public IResourceManager
{
	struct TResource
	{
		IResource *pRes;
		uint refCount;
	};
	std::vector<TResource> _resources;

	struct TDefaultResource
	{
		TDefaultResource(IResource* pResIn, uint refCountIn, DEFAULT_RES_TYPE typeIn) : pRes(pResIn), refCount(refCountIn), type(typeIn) {}

		IResource *pRes{nullptr};
		uint refCount{0};
		DEFAULT_RES_TYPE type{DEFAULT_RES_TYPE::NONE};		
	};
	std::vector<TDefaultResource> _default_resources;
	
	ICoreRender *_pCoreRender{nullptr};
	IFileSystem *_pFilesystem{nullptr};

	CRITICAL_SECTION _cs{};

	static const char* _resourceToStr(IResource* pRes);

	#ifdef USE_FBX
	void _InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void _DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

	bool _FBXLoad(IModel *&pMesh, const char *pFileName, IProgressSubscriber *pPregress);

	bool _LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void _LogSceneHierarchy(IModel *&pModel, FbxScene* pScene);
	void _LogNode(std::vector<ICoreMesh *>& meshes, FbxNode* pNode, int pDepth);
	void _LogMesh(std::vector<ICoreMesh *>& meshes, FbxMesh *pMesh, int tabs);
	void _LogNodeTransform(FbxNode* pNode, int tabs);		
	#endif

public:

	ResourceManager();
	~ResourceManager();

	void Init();
	
	API LoadModel(OUT IModel **pModel, const char *pFileName, IProgressSubscriber *pPregress) override;
	API LoadShaderText(OUT ShaderText *pShader, const char* pVertName, const char* pGeomName, const char* pFragName) override;
	API GetDefaultResource(OUT IResource **pRes, DEFAULT_RES_TYPE type) override;
	API AddToList(IResource *pResource) override;
	API GetNumberOfResources(OUT uint *number) override;
	API GetRefNumber(OUT uint *number, const IResource *pResource) override;
	API DecrementRef(IResource *pResource) override;
	API RemoveFromList(IResource *pResource) override;
	API FreeAllResources() override;
	API GetName(OUT const char **pName) override;
};
