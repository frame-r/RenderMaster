#include "Pch.h"
#include "ResourceManager.h"
#include "Filesystem.h"
#include "Core.h"
#include "Render.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "StructuredBuffer.h"
#include "Camera.h"
#include "ConsoleWindow.h"
#include "SceneManager.h"
#include <memory>


using namespace std;

//#if defined _MSC_VER && _MSC_VER <= 1900
//// VS 2015 ships filesystem as TS
namespace fs = std::experimental::filesystem;
//#else
// for fully C++17 conformant toolchain
//namespace fs = std::filesystem;
//#endif

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

#ifdef USE_FBX

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

void ResourceManager::_FBX_initialize_SDK_objects(FbxManager*& pManager, FbxScene*& pScene)
{
	pManager = FbxManager::Create();

	if (!pManager)
	{
		if (fbxDebug) LOG_FATAL("Error: Unable to create FBX Manager!");
		return;
	}

	if (fbxDebug) LOG_NORMAL_FORMATTED("Autodesk FBX SDK version %s", pManager->GetVersion());

	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");

	if (!pScene)
	{
		LOG_FATAL("Error: Unable to create FBX scene!");
		return;
	}
}

void ResourceManager::_FBX_destroy_SDK_objects(FbxManager* pManager, bool pExitStatus)
{
	if (pManager) pManager->Destroy();
	if (pExitStatus)
		if (fbxDebug) LOG("FBX SDK destroyed");
}

bool ResourceManager::_FBX_load_scene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	int lAnimStackCount;
	bool lStatus;

	// Get the file version number generate by the FBX SDK.
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(pManager, "");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if (!lImportStatus)
	{
		FbxString error = lImporter->GetStatus().GetErrorString();

		LOG_FATAL("Call to FbxImporter::Initialize() failed.");
		LOG_FATAL_FORMATTED("Error returned: %s", error.Buffer());

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			LOG_FORMATTED("FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);
			LOG_FORMATTED("FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	if (fbxDebug) LOG_FORMATTED("FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		if (fbxDebug) LOG_FORMATTED("FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.
		lAnimStackCount = lImporter->GetAnimStackCount();

		if (fbxDebug) LOG("Animation Stack Information:");
		if (fbxDebug) LOG_FORMATTED("Number of Animation Stacks: %d", lAnimStackCount);
		if (fbxDebug) LOG_FORMATTED("Current Animation Stack: \"%s\"", lImporter->GetActiveAnimStackName().Buffer());

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	lStatus = lImporter->Import(pScene);

	if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		LOG_FATAL("No support entering password!");

	lImporter->Destroy();

	return lStatus;
}

void ResourceManager::_FBX_load_scene_hierarchy(vector<IMesh*>& meshes, FbxScene * pScene, const char *pFullPath, const char *pRelativePath)
{
	FbxString lString;

	if (fbxDebug) LOG("Scene hierarchy:");

	FbxNode* lRootNode = pScene->GetRootNode();
	_FBX_load_node(meshes, lRootNode, 0, pFullPath, pRelativePath);

	if (meshes.size() == 0)
		LOG_WARNING("No meshes loaded");
}

void ResourceManager::_FBX_load_node(vector<IMesh*>& meshes, FbxNode* pNode, int depth, const char *fullPath, const char *pRelativePath)
{
	FbxString lString;
	FbxNodeAttribute* node = pNode->GetNodeAttribute();
	FbxNodeAttribute::EType lAttributeType = FbxNodeAttribute::eUnknown;
	if (node) lAttributeType = node->GetAttributeType();

	switch (lAttributeType)
	{
		case FbxNodeAttribute::eMesh:		_FBX_load_mesh(meshes, (FbxMesh*)pNode->GetNodeAttribute(), pNode, fullPath, pRelativePath); break;
		case FbxNodeAttribute::eMarker:		LOG(("(eMarker) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eSkeleton:	LOG(("(eSkeleton) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eNurbs:		LOG(("(eNurbs) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::ePatch:		LOG(("(ePatch) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eCamera:		_FBX_load_node_transform(pNode, ("(eCamera) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLight:		LOG(("(eLight) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLODGroup:	LOG(("(eLODGroup) " + lString + pNode->GetName()).Buffer()); break;
		default:							_FBX_load_node_transform(pNode, ("(unknown!) " + lString + pNode->GetName()).Buffer()); break;
	}

	int childs = pNode->GetChildCount();
	if (childs)
	{
		if (fbxDebug) LOG_FORMATTED("for node=%s childs=%i", pNode->GetName(), childs);
		for (int i = 0; i < childs; i++)
			_FBX_load_node(meshes, pNode->GetChild(i), depth + 1, fullPath, pRelativePath);
	}
}

void add_tabs(FbxString& buff, int tabs)
{
	for (int i = 0; i < tabs; i++)
		buff += " ";
}

void ResourceManager::_FBX_load_mesh(vector<IMesh*>& meshes, FbxMesh *pMesh, FbxNode *pNode, const char *fullPath, const char *pRelativePath)
{
	int control_points_count = pMesh->GetControlPointsCount();
	int polygon_count = pMesh->GetPolygonCount();
	int normal_element_count = pMesh->GetElementNormalCount();
	int uv_layer_count = pMesh->GetElementUVCount();
	int tangent_layers_count = pMesh->GetElementTangentCount();
	int binormal_layers_count = pMesh->GetElementBinormalCount();

	string path = string(pRelativePath) + "#" + string(pNode->GetName());

	FbxVector4 tr = pNode->EvaluateGlobalTransform().GetT();
	FbxVector4 rot = pNode->EvaluateGlobalTransform().GetR();
	FbxVector4 sc = pNode->EvaluateGlobalTransform().GetS();

	if (fbxDebug)
		DEBUG_LOG_FORMATTED("(eMesh) %-10.10s T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f) CP=%5d POLYS=%5d NORMAL=%d UV=%d TANG=%d BINORM=%d", 
		pNode->GetName(),
		tr[0], tr[1], tr[2], rot[0], rot[1], rot[2], sc[0], sc[1], sc[2],
		control_points_count, polygon_count, normal_element_count, uv_layer_count, tangent_layers_count, binormal_layers_count);

	struct Vertex
	{
		float x, y, z, w;
		float nx, ny, nz, n_;
		float tx, ty;
	};

	vector<Vertex> vertecies;
	vertecies.reserve(polygon_count * 3);

	FbxVector4* control_points_array = pMesh->GetControlPoints();
	FbxGeometryElementNormal* pNormals = pMesh->GetElementNormal(0);

	FbxGeometryElementUV *pUV = pMesh->GetElementUV(0);

	int vertex_counter = 0;

	for (int i = 0; i < polygon_count; i++)
	{
		int polygon_size = pMesh->GetPolygonSize(i);

		for (int t = 0; t < polygon_size - 2; t++) // triangulate large polygons
			for (int j = 0; j < 3; j++)
			{
				Vertex v;

				int local_vert_idx = t + j;
				if (j == 0)
					local_vert_idx = 0;

				int ctrl_point_idx = pMesh->GetPolygonVertex(i, local_vert_idx);

				// position
				FbxVector4 lCurrentVertex = control_points_array[ctrl_point_idx];
				v.x = (float)lCurrentVertex[0];
				v.y = (float)lCurrentVertex[1];
				v.z = (float)lCurrentVertex[2];
				v.w = 1.0f;

				// normal
				if (normal_element_count)
				{
					FbxVector4 normal_fbx;

					switch (pNormals->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
						{
							switch (pNormals->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									normal_fbx = pNormals->GetDirectArray().GetAt(ctrl_point_idx);
								break;

								case FbxGeometryElement::eIndexToDirect:
								{
									int id = pNormals->GetIndexArray().GetAt(ctrl_point_idx);
									normal_fbx = pNormals->GetDirectArray().GetAt(id);
								}
								break;

								default: break;
							}
						}
						break;

						case FbxGeometryElement::eByPolygonVertex:
						{
							switch (pNormals->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								normal_fbx = pNormals->GetDirectArray().GetAt(vertex_counter + local_vert_idx);
								break;

							case FbxGeometryElement::eIndexToDirect:
							{
								int id = pNormals->GetIndexArray().GetAt(vertex_counter + local_vert_idx);
								normal_fbx = pNormals->GetDirectArray().GetAt(id);
							}
							break;

							default: break;
							}
						}
						break;

						default: assert(false);
							break;
					}
					
					v.nx = (float)normal_fbx[0];
					v.ny = (float)normal_fbx[1];
					v.nz = (float)normal_fbx[2];
					v.n_ = 0.0f;
				}

				if (uv_layer_count) // TODO
				{
					FbxVector2 uv_fbx;

					switch (pUV->GetMappingMode())
					{
					case FbxGeometryElement::eByControlPoint:
					{
						switch (pUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							uv_fbx = pUV->GetDirectArray().GetAt(ctrl_point_idx);
							break;

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = pUV->GetIndexArray().GetAt(ctrl_point_idx);
							uv_fbx = pUV->GetDirectArray().GetAt(id);
						}
						break;

						default: break;
						}
					}
					break;

					case FbxGeometryElement::eByPolygonVertex:
					{
						switch (pUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							uv_fbx = pUV->GetDirectArray().GetAt(vertex_counter + local_vert_idx);
							break;

						case FbxGeometryElement::eIndexToDirect:
						{
							int id = pUV->GetIndexArray().GetAt(vertex_counter + local_vert_idx);
							uv_fbx = pUV->GetDirectArray().GetAt(id);
						}
						break;

						default: break;
						}
					}
					break;

					default: assert(false);
						break;
					}

					v.tx = (float)uv_fbx[0];
					v.ty = (float)uv_fbx[1];
				}
				
				vertecies.push_back(v);
			}

		vertex_counter += polygon_size;
	}

	MeshDataDesc vertDesc;
	vertDesc.pData = reinterpret_cast<uint8*>(&vertecies[0]);
	vertDesc.numberOfVertex = (uint)vertecies.size();
	vertDesc.positionOffset = 0;
	vertDesc.positionStride = sizeof(Vertex);
	vertDesc.normalsPresented = normal_element_count > 0;
	vertDesc.normalOffset = (normal_element_count > 0) * 16;
	vertDesc.normalStride = (normal_element_count > 0) * sizeof(Vertex);
	vertDesc.texCoordPresented = uv_layer_count > 0;
	vertDesc.texCoordOffset = (uv_layer_count > 0) * 32;
	vertDesc.texCoordStride = (uv_layer_count > 0) * sizeof(Vertex);

	MeshIndexDesc indexDesc;
	indexDesc.pData = nullptr;
	indexDesc.number = 0;

	ICoreMesh *pCoreMesh = nullptr;
	_pCoreRender->CreateMesh((ICoreMesh**)&pCoreMesh, &vertDesc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);

	if (pCoreMesh)
		meshes.push_back(new Mesh(pCoreMesh, path));
	else
		LOG_FATAL("ResourceManager::_FBX_load_mesh(): Can not create mesh");
}

void ResourceManager::_FBX_load_node_transform(FbxNode* pNode, const char *str)
{
	FbxVector4 tr = pNode->EvaluateGlobalTransform().GetT();
	FbxVector4 rot = pNode->EvaluateGlobalTransform().GetR();
	FbxVector4 sc = pNode->EvaluateGlobalTransform().GetS();

	if (fbxDebug)
		DEBUG_LOG_FORMATTED("%s T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", str, tr[0], tr[1], tr[2], rot[0], rot[1], rot[2], sc[0], sc[1], sc[2]);
}
vector<IMesh*> ResourceManager::_FBX_load_meshes(const char *pFullPath, const char *pRelativePath)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;
	vector<IMesh*> meshes;

	if (fbxDebug) LOG("Initializing FBX SDK...");

	_FBX_initialize_SDK_objects(lSdkManager, lScene);

	FbxString lFilePath(pFullPath);

	LOG_FORMATTED("Loading file: %s", lFilePath.Buffer());

	bool lResult = _FBX_load_scene(lSdkManager, lScene, lFilePath.Buffer());

	if (!lResult)
		LOG_FATAL("An error occurred while loading the scene...");
	else
		_FBX_load_scene_hierarchy(meshes, lScene, pFullPath, pRelativePath);

	if (fbxDebug) LOG("Destroying FBX SDK...");
	_FBX_destroy_SDK_objects(lSdkManager, lResult);

	return std::move(meshes);
}
#endif

Render* getRender()
{
	IRender *ret;
	_pCore->GetSubSystem((ISubSystem**)&ret, SUBSYSTEM_TYPE::RENDER);
	return static_cast<Render*>(ret);
}

ResourceManager::ResourceManager()
{
	const char *pString = nullptr;
	_pCore->GetSubSystem((ISubSystem**)&_pFilesystem, SUBSYSTEM_TYPE::FILESYSTEM);

	_pCore->consoleWindow()->addCommand("resources_list", std::bind(&ResourceManager::resources_list, this, std::placeholders::_1, std::placeholders::_2));

	_pCore->AddProfilerCallback(this);
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::ReloadTextFile(ITextFile *shaderText)
{
	const char *pShaderName;
	shaderText->GetFile(&pShaderName);

	DEBUG_LOG_FORMATTED("Reloading shader %s ...", pShaderName);
	const char *t = loadTextFile(pShaderName);

	shaderText->SetText(t);
}

void ResourceManager::Init()
{
	InitializeCriticalSection(&_cs);

	_pCore->GetSubSystem((ISubSystem**)&_pCoreRender, SUBSYSTEM_TYPE::CORE_RENDER);

	LOG("Resource Manager initalized");
}

size_t ResourceManager::sharedResources()
{
	return _sharedMeshes.size() + _sharedTextures.size() + _sharedTextFiles.size();
}

size_t ResourceManager::runtimeResources()
{
	return _runtimeTextures.size() + _runtimeMeshes.size() + _runtimeGameobjects.size() + _runtimeRenderTargets.size() + _runtimeStructuredBuffers.size();
}

API ResourceManager::resources_list(const char **args, uint argsNumber)
{
	LOG_FORMATTED("========= Runtime resources: %i =============", runtimeResources());

	LOG_FORMATTED("Runtime Meshes: %i", _runtimeMeshes.size());
	LOG_FORMATTED("Runtime Textures: %i", _runtimeTextures.size());
	LOG_FORMATTED("Runtime Game Objects: %i", _runtimeGameobjects.size());
	LOG_FORMATTED("Runtime Shaders: %i", _runtimeShaders.size());

	#define PRINT_RUNTIME_RESOURCES(TITLE, SET) \
	LOG(TITLE); \
	for (auto it = SET.begin(); it != SET.end(); it++) \
	{ \
		int refs; \
		(*it)->GetReferences(&refs); \
		LOG_FORMATTED("{refs = %i}", refs); \
	}

	PRINT_RUNTIME_RESOURCES("Meshes:", _runtimeMeshes);
	PRINT_RUNTIME_RESOURCES("Textures:", _runtimeTextures);
	PRINT_RUNTIME_RESOURCES("GameObjects:", _runtimeGameobjects);
	PRINT_RUNTIME_RESOURCES("Shaders:", _runtimeShaders);

	LOG_FORMATTED("========= Shared resources: %i =============", sharedResources());
	LOG_FORMATTED("Shared Meshes: %i", _sharedMeshes.size());
	LOG_FORMATTED("Shared Textures: %i", _sharedTextures.size());
	LOG_FORMATTED("Shared Shader Texts: %i", _sharedTextFiles.size());

	#define PRINT_SHARED_RESOURCES(TITLE, MAP, INTERFACE) \
	LOG(TITLE); \
	for (auto it = MAP.begin(); it != MAP.end(); it++) \
	{ \
		INTERFACE *m = it->second; \
		int refs; \
		m->GetReferences(&refs); \
		const char *path; \
		m->GetFile(&path); \
		LOG_FORMATTED("{refs = %i, file = \"%s\"}", refs, path); \
	}
	
	PRINT_SHARED_RESOURCES("Shared Meshes:", _sharedMeshes, IMesh);
	PRINT_SHARED_RESOURCES("Shared Textures:", _sharedTextures, ITexture);
	PRINT_SHARED_RESOURCES("Shared TextFiles:", _sharedTextFiles, ITextFile);

	return S_OK;
}

uint ResourceManager::getNumLines()
{
	return 4;
}

string ResourceManager::getString(uint i)
{
	switch (i)
	{
		case 0: return "===== Resource Manager =====";
		case 1: return "Shared resources: " + std::to_string(sharedResources());
		case 2: return "Runtime resources: " + std::to_string(runtimeResources());
		case 3: return "";
	};
	assert(0);
	return "";
}

string ResourceManager::constructFullPath(const string& file)
{
	const char *pDataPath;
	_pCore->GetDataDir(&pDataPath);
	string dataPath = string(pDataPath);
	return dataPath + '\\' + file;
}

bool ResourceManager::errorIfPathNotExist(const string& fullPath)
{
	int exist;
	_pFilesystem->FileExist(fullPath.c_str(), &exist);
	if (!exist)
	{
		LOG_WARNING_FORMATTED("File \"%s\" doesn't exist", fullPath.c_str());
	}
	return exist;
}

API ResourceManager::GetName(OUT const char **pName)
{
	*pName = "ResourceManager";
	return S_OK;
}

API ResourceManager::Free()
{
	whiteTetxure->Release();
	whiteTetxure = nullptr;

	assert(_sharedMeshes.size() == 0 &&			"ResourceManager::Free: _sharedMeshes.size() != 0. You should release all meshes before free resource manager");
	assert(_runtimeMeshes.size() == 0 &&		"ResourceManager::Free: _runtimeMeshes.size() != 0. You should release all meshes before free resource manager");
	assert(_sharedTextures.size() == 0 &&		"ResourceManager::Free: _sharedTextures.size() != 0. You should release all textures before free resource manager");
	assert(_runtimeTextures.size() == 0 &&		"ResourceManager::Free: _runtimeTextures.size() != 0. You should release all textures before free resource manager");
	assert(_runtimeGameobjects.size() == 0 &&	"ResourceManager::Free: _runtimeGameobjects.size() != 0. You should release all gameobjects before free resource manager");

	return S_OK;
}

void split_mesh_path(const string& meshPath, string& relativeModelPath, string& meshID)
{
	vector<string> paths = split(string(meshPath), '#');
	if (paths.size() < 2)
		relativeModelPath = meshPath;
	else
	{
		relativeModelPath = paths[0];
		meshID = paths[1];
	}
}

vector<IMesh*> ResourceManager::findLoadedMeshes(const char* pRelativeModelPath, const char *pMeshID)
{
	vector<IMesh*> out;

	for (auto it = _sharedMeshes.begin(); it != _sharedMeshes.end(); it++)
	{
		string path = it->first;
		string relativeModelPath;
		string meshID;
		split_mesh_path(path, relativeModelPath, meshID);

		if (pMeshID == nullptr)
		{
			if (relativeModelPath == pRelativeModelPath)
				out.push_back(it->second);
		}else
		{
			if (relativeModelPath == pRelativeModelPath && meshID == pMeshID)
				out.push_back(it->second);
		}
	}

	return std::move(out);
}

const char* ResourceManager::loadTextFile(const char *pShaderName)
{
	IFile *pFile = nullptr;
	uint fileSize = 0;
	
	const char *pString;
	_pCore->GetInstalledDir(&pString);
	string installedDir = string(pString);
	string shader_path = installedDir + '\\' + SHADER_DIR + '\\' + pShaderName;

	if (!errorIfPathNotExist(shader_path))
		return nullptr;
	
	_pFilesystem->OpenFile(&pFile, shader_path.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

	pFile->FileSize(&fileSize);

	char *tmp = new char[fileSize + 1];
	tmp[fileSize] = '\0';

	pFile->Read((uint8 *)tmp, fileSize);
	pFile->CloseAndFree();

	return tmp;
}

API ResourceManager::LoadModel(OUT IModel **pModel, const char *path)
{
	assert(is_relative(path) && "ResourceManager::LoadModel(): fileName must be relative");

	auto fullPath = constructFullPath(path);

	if (!errorIfPathNotExist(fullPath))
	{
		*pModel = nullptr;
		return E_FAIL;
	}

	vector<IMesh*> loaded_meshes = findLoadedMeshes(path, nullptr);

	IModel *model = nullptr;

	const string file_ext = fileExtension(path);

	if (loaded_meshes.size())
	{
		model = new Model(loaded_meshes);
		uint id;
		model->GetID(&id);

		#ifdef PROFILE_RESOURCES
			DEBUG_LOG_FORMATTED("ResourceManager::LoadModel() new Model %#010x id = %i", model, id);
		#endif

		_runtimeGameobjects.emplace(model);

		*pModel = model;
	} else	

#ifdef USE_FBX
	if (file_ext == "fbx")
	{
		loaded_meshes = _FBX_load_meshes(fullPath.c_str(), path);
		for (IMesh *m : loaded_meshes)
		{
			const char *meshName;
			m->GetFile(&meshName);

			_sharedMeshes.emplace(meshName, m);
		}
		model = new Model(loaded_meshes);
		uint id;
		model->GetID(&id);

		#ifdef PROFILE_RESOURCES
			DEBUG_LOG_FORMATTED("ResourceManager::LoadModel() new Model %#010x id = %i", model, id);
		#endif

		_runtimeGameobjects.emplace(model);

		*pModel = model;
	}
	else
#endif
	{
		LOG_FATAL_FORMATTED("ResourceManager::LoadModel unsupported format \"%s\"", file_ext.c_str());
		return E_FAIL;
	}
	
	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<IModel*>(model));

	return S_OK;
}

API ResourceManager::LoadMesh(OUT IMesh **pMesh, const char *path)
{
	string relativeModelPath;
	string meshID;
	split_mesh_path(path, relativeModelPath, meshID);

	vector<IMesh*> loaded_meshes = findLoadedMeshes(relativeModelPath.c_str(), meshID.c_str());

	if (loaded_meshes.size())
	{
		*pMesh = loaded_meshes[0];
		return S_OK;
	}

	ICoreMesh *stdCoreMesh = nullptr;

	if (!strcmp(path, "std#plane"))
	{
		float vertex[24] =
		{
			-1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 1.0f,
			 1.0f,-1.0f, 0.0f, 1.0f,	1.0f, 0.0f,
			 1.0f, 1.0f, 0.0f, 1.0f,	1.0f, 1.0f,
			-1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f
		};
		
		unsigned short indexPlane[6]
		{
			0, 2, 1,
			0, 1, 3
		};

		MeshDataDesc desc;
		desc.pData = reinterpret_cast<uint8*>(vertex);
		desc.numberOfVertex = 4;
		desc.positionStride = 24;
		desc.texCoordPresented = true;
		desc.texCoordOffset = 16;
		desc.texCoordStride = 24;

		MeshIndexDesc indexDesc;
		indexDesc.pData = reinterpret_cast<uint8*>(indexPlane);
		indexDesc.number = 6;
		indexDesc.format = MESH_INDEX_FORMAT::INT16;

		ThrowIfFailed(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES));

	} else if (!strcmp(path, "std#axes"))
	{
		float vertexAxes[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

		MeshIndexDesc indexEmpty;

		MeshDataDesc descAxes;
		descAxes.pData = reinterpret_cast<uint8*>(vertexAxes);
		descAxes.numberOfVertex = 2;
		descAxes.positionStride = 16;

		ThrowIfFailed(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES));

	} else if (!strcmp(path, "std#axes_arrows"))
	{
		// Layout: position, color, position, color, ...
		const float arrowRadius = 0.052f;
		const float arrowLength = 0.28f;
		const int segments = 12;
		const int numberOfVeretex =  3 * segments;
		const int floats = 4 * numberOfVeretex;

		float vertexAxesArrows[floats];
		{
			for (int j = 0; j < segments; j++)
			{
				constexpr float pi2 = 3.141592654f * 2.0f;
				float alpha = pi2 * (float(j) / segments);
				float dAlpha = pi2 * (1.0f / segments);

				vec4 v1, v2, v3;

				v1.xyzw[0] = 1.0f;
				v1.w = 1.0f;

				v2.xyzw[0] = 1.0f - arrowLength;
				v2.xyzw[1] = cos(alpha) * arrowRadius;
				v2.xyzw[2] = sin(alpha) * arrowRadius;
				v2.w = 1.0f;

				v3.xyzw[0] = 1.0f - arrowLength;
				v3.xyzw[1] = cos(alpha + dAlpha) * arrowRadius;
				v3.xyzw[2] = sin(alpha + dAlpha) * arrowRadius;
				v3.w = 1.0f;

				memcpy(vertexAxesArrows + j * 12 + 0, &v1.x, 16);
				memcpy(vertexAxesArrows + j * 12 + 4, &v2.x, 16);
				memcpy(vertexAxesArrows + j * 12 + 8, &v3.x, 16);
			}
		}
		MeshIndexDesc indexEmpty;

		MeshDataDesc descArrows;
		descArrows.pData = reinterpret_cast<uint8*>(vertexAxesArrows);
		descArrows.numberOfVertex = numberOfVeretex;
		descArrows.positionStride = 16;

		ThrowIfFailed(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descArrows, &indexEmpty, VERTEX_TOPOLOGY::TRIANGLES));

	} else if (!strcmp(path, "std#grid"))
	{
		const float linesInterval = 5.0f;
		const int linesNumber = 31;
		const float startOffset = linesInterval * (linesNumber / 2);

		vec4 vertexGrid[4 * linesNumber];
		for (int i = 0; i < linesNumber; i++)
		{
			vertexGrid[i * 4] = vec4(-startOffset + i * linesInterval, -startOffset, 0.0f, 1.0f);
			vertexGrid[i * 4 + 1] = vec4(-startOffset + i * linesInterval, startOffset, 0.0f, 1.0f);
			vertexGrid[i * 4 + 2] = vec4(startOffset, -startOffset + i * linesInterval, 0.0f, 1.0f);
			vertexGrid[i * 4 + 3] = vec4(-startOffset, -startOffset + i * linesInterval, 0.0f, 1.0f);
		}

		MeshIndexDesc indexEmpty;

		MeshDataDesc descGrid;
		descGrid.pData = reinterpret_cast<uint8*>(vertexGrid);
		descGrid.numberOfVertex = 4 * linesNumber;
		descGrid.positionStride = 16;

		ThrowIfFailed(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descGrid, &indexEmpty, VERTEX_TOPOLOGY::LINES));
	}
	else if (!strcmp(path, "std#quad_lines"))
	{
		vec4 vertex[8];
		vertex[0] = vec4(0.0f, 0.0f, 0.0f, 1.0f); vertex[1] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		vertex[2] = vec4(1.0f, 0.0f, 0.0f, 1.0f); vertex[3] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
		vertex[4] = vec4(1.0f, 1.0f, 0.0f, 1.0f); vertex[5] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		vertex[6] = vec4(0.0f, 1.0f, 0.0f, 1.0f); vertex[7] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		MeshIndexDesc indexEmpty;

		MeshDataDesc descGrid;
		descGrid.pData = reinterpret_cast<uint8*>(vertex);
		descGrid.numberOfVertex = 8;
		descGrid.positionStride = 16;

		ThrowIfFailed(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descGrid, &indexEmpty, VERTEX_TOPOLOGY::LINES));
	}

	if (stdCoreMesh)
	{
		Mesh *m = new Mesh(stdCoreMesh, path);

		#ifdef PROFILE_RESOURCES
			DEBUG_LOG_FORMATTED("ResourceManager::LoadMesh() new Mesh %#010x", m);
		#endif
		*pMesh = m;
		_sharedMeshes.emplace(path, m);
		return S_OK;
	}

	#ifdef USE_FBX

		assert(false); // not impl

		//auto fullPath = constructFullPath(relativeModelPath);

		//if (!errorIfPathNotExist(fullPath))
		//{
		//	*pMesh = nullptr;
		//	return E_FAIL;
		//}

		//const string ext = ToLowerCase(fs::path(relativeModelPath).extension().string().erase(0, 1));
		//if (ext == "fbx")
		//{
		//	loaded_meshes = _FBXLoadMeshes(fullPath.c_str(), relativeModelPath.c_str());

		//	for (IResource *m : loaded_meshes)
		//		_cache_resources.emplace(m);

		//	loaded_meshes.clear();

		//	collect_model_mesh(loaded_meshes, _cache_resources, relativeModelPath.c_str(), meshID.c_str());
		//	if (loaded_meshes.size())
		//	{
		//		// move from _cache_resources to _resources
		//		auto it = _cache_resources.find(loaded_meshes[0]);
		//		_resources.insert(loaded_meshes[0]);
		//		_cache_resources.erase(it);


		//		*pMeshResource = loaded_meshes[0];

		//		return S_OK;
		//	}
		//}
		//else

	#endif
		//{
		//	LOG_FATAL_FORMATTED("ResourceManager::LoadMesh unsupported format \"%s\"", ext.c_str());
		//	return E_FAIL;
		//}

	return S_OK;
}

API ResourceManager::LoadTextFile(OUT ITextFile **pShader, const char *path)
{
	const char *text = loadTextFile(path);

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::LoadTextFile() new TextFile");
	#endif

	TextFile *textFile = new TextFile(text, path);
	_sharedTextFiles.emplace(path, textFile);

	*pShader = textFile;

	return S_OK;
}

#pragma pack(push,1)

const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

struct DDS_PIXELFORMAT
{
	uint32_t size;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t RGBBitCount;
	uint32_t RBitMask;
	uint32_t GBitMask;
	uint32_t BBitMask;
	uint32_t ABitMask;
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME 0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

enum DDS_MISC_FLAGS2
{
	DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT     dxgiFormat;
	uint32_t        resourceDimension;
	uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
	uint32_t        arraySize;
	uint32_t        miscFlags2;
};

#pragma pack(pop)

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif

#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

TEXTURE_FORMAT DDSToEngFormat(const DDS_PIXELFORMAT& ddpf)
{
	if (ddpf.flags & DDS_RGB)
	{
		// Note that sRGB formats are written using the "DX10" extended header

		switch (ddpf.RGBBitCount)
		{
		case 32:
			if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
			{
				return TEXTURE_FORMAT::RGBA8;
			}

			//if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
			//{
			//	//return DXGI_FORMAT_B8G8R8A8_UNORM;
			//	break;
			//}

			//if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
			//{
			//	//return DXGI_FORMAT_B8G8R8X8_UNORM;
			//	break;
			//}

			// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

			// Note that many common DDS reader/writers (including D3DX) swap the
			// the RED/BLUE masks for 10:10:10:2 formats. We assume
			// below that the 'backwards' header mask is being used since it is most
			// likely written by D3DX. The more robust solution is to use the 'DX10'
			// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

			// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
			//if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
			//{
			//	//return DXGI_FORMAT_R10G10B10A2_UNORM;
			//	break;
			//}

			//// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

			//if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
			//{
			//	//return DXGI_FORMAT_R16G16_UNORM;
			//	break;
			//}

			if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
			{
				// Only 32-bit color channel format in D3D9 was R32F
				return TEXTURE_FORMAT::R32F;
			}
			break;

		case 24:
			// No 24bpp DXGI formats aka D3DFMT_R8G8B8

		case 16:
			//if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
			//{
			//	//return DXGI_FORMAT_B5G5R5A1_UNORM;
			//	break;
			//}
			//if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
			//{
			//	//return DXGI_FORMAT_B5G6R5_UNORM;
			//	break;
			//}

			//// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

			//if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
			//{
			//	//return DXGI_FORMAT_B4G4R4A4_UNORM;
			//	break;
			//}

			// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

			// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
			break;
		}
	}
	else if (ddpf.flags & DDS_LUMINANCE)
	{
		if (8 == ddpf.RGBBitCount)
		{
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
			{
				return TEXTURE_FORMAT::R8; // D3DX10/11 writes this out as DX10 extension
			}

			// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
			{
				return TEXTURE_FORMAT::RG8; // Some DDS writers assume the bitcount should be 8 instead of 16
			}
		}

		if (16 == ddpf.RGBBitCount)
		{
			//if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
			//{
			//	//return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
			//}
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
			{
				return TEXTURE_FORMAT::RG8; // D3DX10/11 writes this out as DX10 extension
			}
		}
	}
	//else if (ddpf.flags & DDS_ALPHA)
	//{
	//	if (8 == ddpf.RGBBitCount)
	//	{
	//		//return DXGI_FORMAT_A8_UNORM;
	//	}
	//}
	else if (ddpf.flags & DDS_BUMPDUDV)
	{
		//if (16 == ddpf.RGBBitCount)
		//{
		//	if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
		//	{
		//		//return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
		//	}
		//}

		//if (32 == ddpf.RGBBitCount)
		//{
		//	if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
		//	{
		//		//return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
		//	}
		//	if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
		//	{
		//		//return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
		//	}

		//	// No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
		//}
	}
	else if (ddpf.flags & DDS_FOURCC)
	{
		if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
		{
			return TEXTURE_FORMAT::DXT1;
		}
		if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
		{
			return TEXTURE_FORMAT::DXT3;
		}
		if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
		{
			return TEXTURE_FORMAT::DXT5;
		}

		// While pre-multiplied alpha isn't directly supported by the DXGI formats,
		// they are basically the same as these BC formats so they can be mapped
		//if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC2_UNORM;
		//}
		//if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC3_UNORM;
		//}

		//if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC4_UNORM;
		//}
		//if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC4_UNORM;
		//}
		//if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC4_SNORM;
		//}

		//if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC5_UNORM;
		//}
		//if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC5_UNORM;
		//}
		//if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_BC5_SNORM;
		//}

		// BC6H and BC7 are written using the "DX10" extended header

		//if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_R8G8_B8G8_UNORM;
		//}
		//if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_G8R8_G8B8_UNORM;
		//}

		//if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
		//{
		//	//return DXGI_FORMAT_YUY2;
		//}

		// Check for D3DFORMAT enums being set here
		switch (ddpf.fourCC)
		{
		case 36: // D3DFMT_A16B16G16R16
			//return DXGI_FORMAT_R16G16B16A16_UNORM;
			break;

		case 110: // D3DFMT_Q16W16V16U16
			return TEXTURE_FORMAT::RGBA16F;

		case 111: // D3DFMT_R16F
			return TEXTURE_FORMAT::R16F;

		case 112: // D3DFMT_G16R16F
			return TEXTURE_FORMAT::RG16F;

		case 113: // D3DFMT_A16B16G16R16F
			//return TEXTURE_FORMAT::RGBA16F;

		case 114: // D3DFMT_R32F
			return TEXTURE_FORMAT::R32F;

		case 115: // D3DFMT_G32R32F
			return TEXTURE_FORMAT::RG32F;

		case 116: // D3DFMT_A32B32G32R32F
			//return TEXTURE_FORMAT::RGBA32F;
			break;
		}
	}

	return TEXTURE_FORMAT::UNKNOWN;
}

ICoreTexture* ResourceManager::loadDDS(const char *path, TEXTURE_CREATE_FLAGS flags)
{
	IFile *pFile = nullptr;
	uint fileSize = 0;

	const char *pString;
	_pCore->GetDataDir(&pString);
	string dataDir = string(pString);
	string fullPath = dataDir + '\\' + path;

	if (!errorIfPathNotExist(fullPath))
		return nullptr;

	_pFilesystem->OpenFile(&pFile, fullPath.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

	pFile->FileSize(&fileSize);

	unique_ptr<uint8[]> fileData = std::make_unique<uint8[]>(fileSize);

	pFile->Read((uint8 *)fileData.get(), fileSize);
	pFile->CloseAndFree();

	// Check magic
	uint32_t dwMagicNumber = *reinterpret_cast<const uint32_t*>(fileData.get());
	if (dwMagicNumber != DDS_MAGIC)
	{
		LOG_WARNING("ResourceManager::loadDDS(): Wrong magic");
		return nullptr;
	}

	const DDS_HEADER* header = reinterpret_cast<const DDS_HEADER*>(fileData.get() + sizeof(uint32_t));

	// Check header sizes
	if (header->size != sizeof(DDS_HEADER) || header->ddspf.size != sizeof(DDS_PIXELFORMAT))
	{
		LOG_WARNING("ResourceManager::loadDDS(): Wrong header sizes");
		return nullptr;
	}

	// Check for DX10 extension
	bool bDXT10Header = (header->ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC);

	ptrdiff_t offset = sizeof(uint32_t) + sizeof(DDS_HEADER) + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
	uint8 *imageData = fileData.get() + offset;
	size_t imageSize = fileSize - offset;

	// Not supported:
	// -DXT10 extension
	// -volume
	// -cubemaps
	if (bDXT10Header ||
		header->flags & DDS_HEADER_FLAGS_VOLUME ||
		header->caps2 & DDS_CUBEMAP)
	{
		LOG_WARNING("ResourceManager::loadDDS(): type not supported");
		return nullptr;
	}

	unique_ptr<uint8[]> imageDataRempped;

	// format
	TEXTURE_FORMAT format = DDSToEngFormat(header->ddspf);

	// Special case: RGB -> RGBA
	if ((header->ddspf.flags & DDS_RGB) && header->ddspf.RGBBitCount == 24)
	{
		assert(imageSize % 3 == 0);
		size_t alphaChannelSize = imageSize / 3;
		size_t elements = header->width * header->height;

		imageDataRempped = std::move(std::make_unique<uint8[]>(imageSize + alphaChannelSize));

		uint8* ptr_src = imageData;
		uint8* ptr_dst = imageDataRempped.get();

		for (size_t i = 0u; i < elements; ++i)
		{
			memcpy(ptr_dst, ptr_dst, 3);
			memset((ptr_dst + 3), 255, 1);

			ptr_dst += 4;
			ptr_src += 3;
		}

		imageData = imageDataRempped.get();
		format = TEXTURE_FORMAT::RGBA8;
	}

	if (format == TEXTURE_FORMAT::UNKNOWN)
	{
		LOG_WARNING("ResourceManager::loadDDS(): format not supported");
		return nullptr;
	}

	const TEXTURE_TYPE type = TEXTURE_TYPE::TYPE_2D;
	bool mipmapsPresented = header->mipMapCount > 1;
	ICoreTexture *tex = nullptr;

	if (FAILED(_pCoreRender->CreateTexture(&tex, imageData, header->width, header->height, type, format, flags, mipmapsPresented)))
	{
		LOG_WARNING("ResourceManager::loadDDS(): failed to create texture");
		return nullptr;
	}

	return tex;
}

API ResourceManager::LoadTexture(OUT ITexture **pTexture, const char *path, TEXTURE_CREATE_FLAGS flags)
{
	ICoreTexture *coreTex;

	if (!strcmp(path, "std#white_texture"))
	{
		if (!whiteTetxure)
		{
			uint8 data[4] = { 255u, 255u, 255u, 255u };
			ThrowIfFailed(_pCoreRender->CreateTexture(&coreTex, data, 1, 1, TEXTURE_TYPE::TYPE_2D, TEXTURE_FORMAT::RGBA8, TEXTURE_CREATE_FLAGS(), false));

			ITexture *tex = new Texture(coreTex, path);

			#ifdef PROFILE_RESOURCES
				DEBUG_LOG_FORMATTED("ResourceManager::CreateTexture() new Texture %#010x", tex);
			#endif

			_sharedTextures.emplace(path, tex);

			*pTexture = tex;
			whiteTetxure = tex;
			whiteTetxure->AddRef();

			return S_OK;
		}
	}
	else
	{
		if (path == NULL || strlen(path) == 0 || !errorIfPathNotExist(path))
		{
			*pTexture = nullptr;
			return E_INVALIDARG;
		}

		const string ext = fileExtension(path);

		if (ext != "dds")
		{
			LOG_WARNING_FORMATTED("Extension %s is not supported", ext.c_str());
			*pTexture = nullptr;
			return E_INVALIDARG;
		}

		coreTex = loadDDS(path, flags);

		if (!coreTex)
		{
			LOG_FATAL("ResourceManager::LoadTexture(): some error occured");
			*pTexture = nullptr;
			return E_INVALIDARG;
		}
	}

	ITexture *tex = new Texture(coreTex, path);

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateTexture() new Texture %#010x", tex);
	#endif

	_sharedTextures.emplace(path, tex);
	*pTexture = tex;

	return S_OK;
}

API ResourceManager::CreateGameObject(OUT IGameObject **pGameObject)
{
	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateGameObject() new GameObject");
	#endif

	IGameObject *g = new GameObject;

	_runtimeGameobjects.emplace(g);
	*pGameObject = g;

	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<IGameObject*>(g));

	return S_OK;
}

API ResourceManager::CreateModel(OUT IModel **pModel)
{
	IModel *g = new Model;

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateModel() new Model");
	#endif

	_runtimeGameobjects.emplace(g);
	*pModel = g;

	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<IModel*>(g));

	return S_OK;
}

API ResourceManager::CreateCamera(OUT ICamera **pCamera)
{
	ICamera *g = new Camera;

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateCamera() new Camera");
	#endif

	_runtimeGameobjects.emplace(g);
	*pCamera = g;

	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<ICamera*>(g));

	return S_OK;
}

API ResourceManager::CreateTexture(OUT ITexture **pTextureOut, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags)
{
	ICoreTexture *pCoreTex;

	if (FAILED(_pCoreRender->CreateTexture(&pCoreTex, nullptr, width, height, type, format, flags, false)))
	{
		*pTextureOut = nullptr;
		LOG_WARNING("ResourceManager::CreateTexture(): failed to create texture");
		return E_FAIL;
	}

	Texture *tex = new Texture(pCoreTex);

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateTexture() new Texture %#010x", tex);
	#endif

	_runtimeTextures.emplace(tex);
	*pTextureOut = tex;

	return S_OK;
}

API ResourceManager::CreateShader(OUT IShader **pShaderOut, const char *vert, const char *geom, const char *frag)
{
	ICoreShader *coreShader = nullptr;

	auto hr = _pCoreRender->CreateShader(&coreShader, vert, frag, geom);
	bool compiled = SUCCEEDED(hr) && coreShader != nullptr;

	if (!compiled)
	{
		*pShaderOut = nullptr;
		const char *shaderText;

		switch (hr)
		{
			case E_VERTEX_SHADER_FAILED_COMPILE: shaderText = vert; break;
			case E_GEOM_SHADER_FAILED_COMPILE: shaderText = geom; break;
			case E_FRAGMENT_SHADER_FAILED_COMPILE: shaderText = frag; break;
		};

		IFile *pFile;
	
		_pFilesystem->OpenFile(&pFile, "err_compile.shader", FILE_OPEN_MODE::WRITE);

		pFile->Write((uint8 *)shaderText, (uint)strlen(shaderText));

		pFile->CloseAndFree();

		#ifdef PROFILE_RESOURCES
			LOG_FATAL_FORMATTED("ResourceManager::CreateShader(): failed to create shader. Shader saved to \"err_compile.shader\"");
		#endif

		return E_FAIL;
	}

	IShader *s = new Shader(coreShader, vert, geom, frag);

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateShader() new Shader %#010x", s);
	#endif

	_runtimeShaders.emplace(s);
	*pShaderOut = s;

	return S_OK;
}

API ResourceManager::CreateRenderTarget(OUT IRenderTarget **pRenderTargetOut)
{
	ICoreRenderTarget *coreRenderTarget = nullptr;

	bool created = SUCCEEDED(_pCoreRender->CreateRenderTarget(&coreRenderTarget)) && coreRenderTarget != nullptr;

	if (!created)
	{
		*pRenderTargetOut = nullptr;
		LOG_WARNING("ResourceManager::CreateRenderTarget(): failed to create constnt buffer");
		return E_FAIL;
	}

	IRenderTarget *rt = new RenderTarget(coreRenderTarget);

	#ifdef PROFILE_RESOURCES
		DEBUG_LOG_FORMATTED("ResourceManager::CreateRenderTarget() new RenderTarget %#010x", rt);
	#endif

	_runtimeRenderTargets.emplace(rt);
	*pRenderTargetOut = rt;

	return S_OK;
}

API ResourceManager::CreateStructuredBuffer(OUT IStructuredBuffer **pBufOut, uint bytes, uint elementSize)
{
	ICoreStructuredBuffer *coreStructuredBuffer = nullptr;

	bool created = SUCCEEDED(_pCoreRender->CreateStructuredBuffer(&coreStructuredBuffer, bytes, elementSize)) && coreStructuredBuffer != nullptr;

	if (!created)
	{
		*pBufOut = nullptr;
		LOG_WARNING("ResourceManager::CreateStructuredBuffer(): failed to create constnt buffer");
		return E_FAIL;
	}

	StructuredBuffer *b = new StructuredBuffer(coreStructuredBuffer);

#ifdef PROFILE_RESOURCES
	DEBUG_LOG_FORMATTED("ResourceManager::CreateStructuredBuffer() new StructuredBuffer %#010x", b);
#endif

	_runtimeStructuredBuffers.emplace(b);
	*pBufOut = b;

	return S_OK;
}


///////////////////////
// Text File
//////////////////////

SHARED_ONLY_RESOURCE_IMPLEMENTATION(TextFile, _pCore, RemoveSharedTextFile)

TextFile::~TextFile()
{
	delete[] text;
}

API TextFile::SetText(const char * textIn)
{
	if (text) delete[] text;
	text = textIn;
	return S_OK;
}

API TextFile::Reload()
{
	IResourceManager *irm = getResourceManager(_pCore);
	ResourceManager *rm = static_cast<ResourceManager*>(irm);
	rm->ReloadTextFile(this);
	return S_OK;
}

