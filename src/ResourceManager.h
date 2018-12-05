#pragma once
#include "Common.h"

#ifdef USE_FBX
#include <fbxsdk.h>
#endif


class ResourceManager final : public IResourceManager
{
	// Rintime resources
	// No files associated with these resources
	std::unordered_set<ITexture*> _runtimeTextures;
	std::unordered_set<IMesh*> _runtimeMeshes;
	std::unordered_set<IGameObject*> _runtimeGameobjects;
	std::unordered_set<IShader*> _runtimeShaders;
	std::unordered_set<IRenderTarget*> _runtimeRenderTargets;

	// Shared resources
	// Maps "file name" -> "pointer"
	std::unordered_map<string, ITexture*> _sharedTextures;	
	std::unordered_map<string, IMesh*> _sharedMeshes;
	std::unordered_map<string, IShaderFile*> _sharedShaderTexts;		
		
	ICoreRender *_pCoreRender = nullptr;
	IFileSystem *_pFilesystem = nullptr;

	CRITICAL_SECTION _cs{};

	#ifdef USE_FBX
	const int fbxDebug = 0;

	void _FBX_initialize_SDK_objects(FbxManager*& pManager, FbxScene*& pScene);
	void _FBX_destroy_SDK_objects(FbxManager* pManager, bool pExitStatus);

	vector<IMesh*> _FBX_load_meshes(const char *pFullPath, const char *pRelativePath);
	bool _FBX_load_scene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void _FBX_load_scene_hierarchy(vector<IMesh*>& meshes, FbxScene* pScene, const char *pFullPath, const char *pRelativePath);
	void _FBX_load_node(vector<IMesh*>& meshes, FbxNode* pNode, int pDepth, const char *pFullPath, const char *pRelativePath);
	void _FBX_load_mesh(vector<IMesh*>& meshes, FbxMesh *pMesh, FbxNode *pNode, const char *pFullPath, const char *pRelativePath);
	void _FBX_load_node_transform(FbxNode* pNode, const char *str);
	#endif

	API resources_list(const char **args, uint argsNumber);

	string construct_full_path(const string& file);
	bool error_if_path_not_exist(const string& fullPath);
	vector<IMesh*> find_loaded_meshes(const char* pRelativeModelPath, const char *pMeshID);
	const char *load_shader(const char *fileName);

public:

	ResourceManager();
	virtual ~ResourceManager();

	void RemoveRuntimeMesh(IMesh *mesh) { _runtimeMeshes.erase(mesh); }
	void RemoveSharedMesh(const string& file) { _sharedMeshes.erase(file); }
	void RemoveRuntimeTexture(ITexture *tex) { _runtimeTextures.erase(tex); }
	void RemoveSharedTexture(const string& file) { _sharedTextures.erase(file); }
	void RemoveRuntimeGameObject(IGameObject *g) { _runtimeGameobjects.erase(g); }
	void RemoveSharedShaderFile(const string& file) { _sharedShaderTexts.erase(file); }
	void RemoveRuntimeShader(IShader *s) { _runtimeShaders.erase(s); }
	void RemoveRuntimeRenderTarget(IRenderTarget *rt) { _runtimeRenderTargets.erase(rt); }

	void ReloadShaderFile(IShaderFile *shaderText);

	void Init();

	API LoadModel(OUT IModel **pModel, const char *pModelPath) override;
	API LoadMesh(OUT IMesh **pMesh, const char *pMeshPath) override;
	API LoadTexture(OUT ITexture **pTexture, const char *pTexturePath, TEXTURE_CREATE_FLAGS flags) override;
	API LoadShaderFile(OUT IShaderFile **pShader, const char *pShaderName) override;

	API CreateTexture(OUT ITexture **pTextureOut, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags) override;
	API CreateShader(OUT IShader **pShaderOut, const char *vert, const char *geom, const char *frag) override;
	API CreateRenderTarget(OUT IRenderTarget **pRenderTargetOut) override;
	API CreateGameObject(OUT IGameObject **pGameObject) override;
	API CreateModel(OUT IModel **pModel) override;
	API CreateCamera(OUT ICamera **pCamera) override;

	API Free() override;

	API GetName(OUT const char **pTxt) override;
};

