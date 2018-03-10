#pragma once
#include "Common.h"
#include <vector>
#include <map>

#ifdef USE_FBX
#include <fbxsdk.h>
#endif

class ResourceManager : public IResourceManager
{
	ICoreRender *_pCoreRender;
	CRITICAL_SECTION _cs;

	std::map<DEFAULT_RESOURCE_TYPE, ICoreMesh*> _default_meshes;

	struct TResource
	{
		IResource *pRes;
		uint refCount;
	};
	std::vector<TResource> _res_vec;

#ifdef USE_FBX
	char buffer[200];

	void _InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void _DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

	bool _LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);

	void _LogSceneHierarchy(FbxScene* pScene);
	void _LogNode(FbxNode* pNode, int pDepth);
	void _LogMesh(FbxMesh *pMesh, int tabs);
	void _LogNodeTransform(FbxNode* pNode, int tabs);

	bool _FBXLoad(IModel *&pMesh, const char *pFileName, IProgressSubscriber *pPregress);
#endif

	static const char* resourceToStr(IResource* pRes);

public:

	ResourceManager();
	~ResourceManager();

	void Init();

	// ISubSystem
	API GetName(const char *&pTxt) override;
	
	// IResourceManager
	API LoadModel(IModel *&pModel, const char *pFileName, IProgressSubscriber *pPregress) override;
	API LoadShader(ICoreShader *&pShader, const char* pVertName, const char* pGeomName, const char* pFragName) override;
	API CreateDefaultModel(IModel *&pModel, DEFAULT_RESOURCE_TYPE type) override;

	API AddToList(IResource *pResource) override;
	API GetRefNumber(IResource *pResource, uint& number) override;
	API DecrementRef(IResource *pResource) override;
	API RemoveFromList(IResource *pResource) override;

	API FreeAllResources() override;
};