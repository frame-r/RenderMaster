#pragma once
#include "Common.h"

#ifdef USE_FBX
#include <fbxsdk.h>
#endif

class TextFile : public BaseResource<ITextFile>
{
	const char *text = nullptr;

public:
	TextFile(const char *textIn, const string& path) :
		text(textIn) {
		_file = path;
	}

	virtual ~TextFile();

	API GetText(OUT const char **textOut) override { *textOut = text; return S_OK; }
	API SetText(const char *textIn) override;
	void Reload() override;
};

class ResourceManager final : public IResourceManager, IProfilerCallback
{
	// Rintime resources
	// No files associated with these resources
	std::unordered_set<ITexture*> _runtimeTextures;
	std::unordered_set<IMesh*> _runtimeMeshes;
	std::unordered_set<IGameObject*> _runtimeGameobjects;
	std::unordered_set<IShader*> _runtimeShaders;
	std::unordered_set<IRenderTarget*> _runtimeRenderTargets;
	std::unordered_set<IStructuredBuffer*> _runtimeStructuredBuffers;

	// Shared resources
	// Maps "file name" -> "pointer"
	std::unordered_map<string, ITexture*> _sharedTextures;
	std::unordered_map<string, IMesh*> _sharedMeshes;
	std::unordered_map<string, ITextFile*> _sharedTextFiles;
		
	ICoreRender *_pCoreRender = nullptr;
	IFileSystem *_pFilesystem = nullptr;

	CRITICAL_SECTION _cs{};

	ITexture *whiteTetxure = nullptr;

	#ifdef USE_FBX
	const int fbxDebug = 1;

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

	uint getNumLines() override;
	string getString(uint i) override;

	string constructFullPath(const string& file);
	bool errorIfPathNotExist(const string& fullPath);
	vector<IMesh*> findLoadedMeshes(const char* pRelativeModelPath, const char *pMeshID);
	const char *loadTextFile(const char *fileName);
	ICoreTexture *loadDDS(const char *pTexturePath, TEXTURE_CREATE_FLAGS flags);
	size_t sharedResources();
	size_t runtimeResources();

	template<class T>
	void tryRemoveFromRuntimes(IBaseResource *res, std::unordered_set<T*>& runtimes)
	{
		if (dynamic_cast<T*>(res))
		{
			const char *path;
			res->GetFile(&path);
			if (*path == '\0')
				runtimes.erase(dynamic_cast<T*>(res));
		}
	}

	template<class T>
	void tryRemoveFromShared(IBaseResource *res, std::unordered_map<string, T*>& shared)
	{
		if (dynamic_cast<T*>(res))
		{
			const char *path;
			res->GetFile(&path);
			if (*path != '\0')
				shared.erase(string(path));
		}
	}

public:

	ResourceManager();
	virtual ~ResourceManager();

	void RemoveResource(IBaseResource *res);
	void ReloadTextFile(ITextFile *shaderText);

	void Init();

	API LoadModel(OUT IModel **pModel, const char *path) override;
	API LoadMesh(OUT IMesh **pMesh, const char *path) override;
	API LoadTexture(OUT ITexture **pTexture, const char *path, TEXTURE_CREATE_FLAGS flags) override;
	API LoadTextFile(OUT ITextFile **pShader, const char *path) override;

	API CreateTexture(OUT ITexture **pTextureOut, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags) override;
	API CreateShader(OUT IShader **pShaderOut, const char *vert, const char *geom, const char *frag) override;
	API CreateRenderTarget(OUT IRenderTarget **pRenderTargetOut) override;
	API CreateStructuredBuffer(OUT IStructuredBuffer **pBufOut, uint size, uint elementSize) override;
	API CreateGameObject(OUT IGameObject **pGameObject) override;
	API CreateModel(OUT IModel **pModel) override;
	API CreateCamera(OUT ICamera **pCamera) override;

	API Free() override;

	API GetName(OUT const char **pTxt) override;
};
