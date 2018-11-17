#pragma once
#include "Common.h"

#ifdef USE_FBX
#include <fbxsdk.h>
#endif


class ResourceManager final : public IResourceManager
{
	std::unordered_set<ITexture*> _runtime_textures;
	std::unordered_map<string, ITexture*> _shared_textures;

	std::unordered_set<IMesh*> _runtime_meshes;
	std::unordered_map<string, IMesh*> _shared_meshes;

	std::unordered_set<IGameObject*> _runtime_gameobjects;

	std::unordered_map<string, IShaderText*> _shared_shadertexts;

	std::unordered_set<IConstantBuffer*> _runtime_constntbuffer;
	std::unordered_set<IShader*> _runtime_shaders;
		
	ICoreRender *_pCoreRender{nullptr};
	IFileSystem *_pFilesystem{nullptr};

	CRITICAL_SECTION _cs{};

	#ifdef USE_FBX
	const int fbxDebug = 0;

	void _InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void _DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

	vector<IMesh*> _FBXLoadMeshes(const char *pFullPath, const char *pRelativePath);
	bool _LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void _LoadSceneHierarchy(vector<IMesh*>& meshes, FbxScene* pScene, const char *pFullPath, const char *pRelativePath);
	void _LoadNode(vector<IMesh*>& meshes, FbxNode* pNode, int pDepth, const char *pFullPath, const char *pRelativePath);
	void _LoadMesh(vector<IMesh*>& meshes, FbxMesh *pMesh, FbxNode *pNode, const char *pFullPath, const char *pRelativePath);
	void _LoadNodeTransform(FbxNode* pNode, const char *str);
	#endif

	API resources_list(const char **args, uint argsNumber);

	string construct_full_path(const string& file);
	bool error_if_path_not_exist(const string& fullPath);
	vector<IMesh*> find_loaded_meshes(const char* pRelativeModelPath, const char *pMeshID);

public:

	ResourceManager();
	virtual ~ResourceManager();

	void RemoveRuntimeMesh(IMesh *mesh) { _runtime_meshes.erase(mesh); }
	void RemoveSharedMesh(const string& file) { _shared_meshes.erase(file); }
	void RemoveRuntimeTexture(ITexture *tex) { _runtime_textures.erase(tex); }
	void RemoveSharedTexture(const string& file) { _shared_textures.erase(file); }
	void RemoveRuntimeGameObject(IGameObject *g) { _runtime_gameobjects.erase(g); }
	void RemoveSharedShaderText(const string& file) { _shared_shadertexts.erase(file); }
	void RemoveRuntimeConstantBuffer(IConstantBuffer *cb) { _runtime_constntbuffer.erase(cb); }
	void RemoveRuntimeShader(IShader *s) { _runtime_shaders.erase(s); }

	void Init();

	API LoadModel(OUT IModel **pModel, const char *pModelPath) override;
	API LoadMesh(OUT IMesh **pMesh, const char *pMeshPath) override;
	API LoadTexture(OUT ITexture **pTexture, const char *pMeshPath, TEXTURE_CREATE_FLAGS flags) override;
	API LoadShaderText(OUT IShaderText **pShader, const char *pVertName, const char *pGeomName, const char *pFragName) override;
	
	API CreateShader(OUT IShader **pShaderOut, const char *vert, const char *geom, const char *frag) override;
	API CreateConstantBuffer(OUT IConstantBuffer **pConstntBuffer, uint size) override;
	API CreateGameObject(OUT IGameObject **pGameObject) override;
	API CreateModel(OUT IModel **pModel) override;
	API CreateCamera(OUT ICamera **pCamera) override;

	API Free() override;
	API GetName(OUT const char **pTxt) override;
};

