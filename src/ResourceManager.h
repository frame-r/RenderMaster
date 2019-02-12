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

	API_RESULT GetText(OUT const char **textOut) override { *textOut = text; return S_OK; }
	API_RESULT SetText(const char *textIn) override;
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

	std::unordered_map<string, unique_ptr<IMaterial>> _materials;
		
	ICoreRender *_pCoreRender = nullptr;
	IFileSystem *_pFilesystem = nullptr;

	CRITICAL_SECTION _cs{};

	ITexture *whiteTetxure = nullptr;

	#ifdef USE_FBX
	const int fbxDebug = 1;

	void _FBX_initialize_SDK_objects(FbxManager*& pManager, FbxScene*& pScene);
	void _FBX_destroy_SDK_objects(FbxManager* pManager, bool pExitStatus);

	struct FBXLoadMesh
	{
		IMesh *mesh;
		mat4 transform;
		vector<FBXLoadMesh*> childs;
	};

	vector<FBXLoadMesh> _FBX_load_meshes(const char *pFullPath, const char *pRelativePath);
	bool _FBX_load_scene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void _FBX_load_scene_hierarchy(vector<FBXLoadMesh>& meshes, FbxScene* pScene, const char *pFullPath, const char *pRelativePath);
	void _FBX_load_node(vector<FBXLoadMesh>& meshes, FbxNode* pNode, int pDepth, const char *pFullPath, const char *pRelativePath);
	void _FBX_load_mesh(vector<FBXLoadMesh>& meshes, FbxMesh *pMesh, FbxNode *pNode, const char *pFullPath, const char *pRelativePath);
	void _FBX_load_node_transform(FbxNode* pNode, const char *str);
	#endif

	API_RESULT resources_list(const char **args, uint argsNumber);

	// IProfilerCallback
	uint getNumLines() override;
	string getString(uint i) override;

	string constructFullPath(const string& file);
	bool errorIfPathNotExist(const string& fullPath);
	IMesh* findLoadedMeshes(const char* pRelativeModelPath);
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

	API_RESULT _LoadModel(OUT IModel **pModel, const char *path) override;
	API_RESULT _LoadTexture(OUT ITexture **pTexture, const char *path, TEXTURE_CREATE_FLAGS flags) override;
	API_RESULT _LoadTextFile(OUT ITextFile **pShader, const char *path) override;
	API_RESULT _LoadStandardMesh(OUT IMesh **pMesh, const char *id) override;

	API_RESULT _CreateMaterial(OUT IMaterial **pMat, const char *name) override;
	API_RESULT _FindMaterial(OUT IMaterial **pMat, const char *name) override;

	API_RESULT _CreateTexture(OUT ITexture **pTextureOut, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags) override;
	API_RESULT _CreateShader(OUT IShader **pShaderOut, const char *vert, const char *geom, const char *frag) override;
	API_RESULT _CreateRenderTarget(OUT IRenderTarget **pRenderTargetOut) override;
	API_RESULT _CreateStructuredBuffer(OUT IStructuredBuffer **pBufOut, uint size, uint elementSize) override;
	API_RESULT _CreateGameObject(OUT IGameObject **pGameObject) override;
	API_RESULT _CreateModel(OUT IModel **pModel) override;
	API_RESULT _CreateCamera(OUT ICamera **pCamera) override;

	API_RESULT _CloneGameObject(OUT IGameObject **pDest, IGameObject *pSrc) override;

	API_RESULT Free() override;

	API_RESULT GetName(OUT const char **pTxt) override;
};
