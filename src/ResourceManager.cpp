#include "pch.h"
#include "ResourceManager.h"
#include "Filesystem.h"
#include "Core.h"
#include "Model.h"
#include "Mesh.h"
#include "Shader.h"
#include "ShaderText.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "Console.h"
#include "SceneManager.h"


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

void ResourceManager::_InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
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

void ResourceManager::_DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
	if (pManager) pManager->Destroy();
	if (pExitStatus)
		if (fbxDebug) LOG("FBX SDK destroyed");
}

bool ResourceManager::_LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
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

void ResourceManager::_LoadSceneHierarchy(vector<IMesh*>& meshes, FbxScene * pScene, const char *pFullPath, const char *pRelativePath)
{
	FbxString lString;

	if (fbxDebug) LOG("Scene hierarchy:");

	FbxNode* lRootNode = pScene->GetRootNode();
	_LoadNode(meshes, lRootNode, 0, pFullPath, pRelativePath);

	if (meshes.size() == 0)
		LOG_WARNING("No meshes loaded");
}

void ResourceManager::_LoadNode(vector<IMesh*>& meshes, FbxNode* pNode, int depth, const char *fullPath, const char *pRelativePath)
{
	FbxString lString;
	FbxNodeAttribute* node = pNode->GetNodeAttribute();
	FbxNodeAttribute::EType lAttributeType = FbxNodeAttribute::eUnknown;
	if (node) lAttributeType = node->GetAttributeType();

	switch (lAttributeType)
	{
		case FbxNodeAttribute::eMesh:		_LoadMesh(meshes, (FbxMesh*)pNode->GetNodeAttribute(), pNode, fullPath, pRelativePath); break;
		case FbxNodeAttribute::eMarker:		LOG(("(eMarker) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eSkeleton:	LOG(("(eSkeleton) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eNurbs:		LOG(("(eNurbs) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::ePatch:		LOG(("(ePatch) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eCamera:		_LoadNodeTransform(pNode, ("(eCamera) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLight:		LOG(("(eLight) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLODGroup:	LOG(("(eLODGroup) " + lString + pNode->GetName()).Buffer()); break;
		default:							_LoadNodeTransform(pNode, ("(unknown!) " + lString + pNode->GetName()).Buffer()); break;
	}

	int childs = pNode->GetChildCount();
	if (childs)
	{
		if (fbxDebug) LOG_FORMATTED("for node=%s childs=%i", pNode->GetName(), childs);
		for (int i = 0; i < childs; i++)
			_LoadNode(meshes, pNode->GetChild(i), depth + 1, fullPath, pRelativePath);
	}
}

void add_tabs(FbxString& buff, int tabs)
{
	for (int i = 0; i < tabs; i++)
		buff += " ";
}

void ResourceManager::_LoadMesh(vector<IMesh*>& meshes, FbxMesh *pMesh, FbxNode *pNode, const char *fullPath, const char *pRelativePath)
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
		float n_x, n_y, n_z, n_zero;
	};

	vector<Vertex> vertecies;
	vertecies.reserve(polygon_count * 3);

	FbxVector4* control_points_array = pMesh->GetControlPoints();
	FbxGeometryElementNormal* pNormals = pMesh->GetElementNormal(0);

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
								default:
								break;
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
							default:
								break;
							}

						}
						break;

						default: assert(false);
							break;
					}
					
					v.n_x = (float)normal_fbx[0];
					v.n_y = (float)normal_fbx[1];
					v.n_z = (float)normal_fbx[2];
					v.n_zero = 0.0f;
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

	MeshIndexDesc indexDesc;
	indexDesc.pData = nullptr;
	indexDesc.number = 0;

	ICoreMesh *pCoreMesh = nullptr;
	_pCoreRender->CreateMesh((ICoreMesh**)&pCoreMesh, &vertDesc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);

	if (pCoreMesh)
		meshes.push_back(new Mesh(pCoreMesh, 1, path));
	else
		LOG_FATAL("ResourceManager::_LoadMesh(): Can not create mesh");
}

void ResourceManager::_LoadNodeTransform(FbxNode* pNode, const char *str)
{
	FbxVector4 tr = pNode->EvaluateGlobalTransform().GetT();
	FbxVector4 rot = pNode->EvaluateGlobalTransform().GetR();
	FbxVector4 sc = pNode->EvaluateGlobalTransform().GetS();

	if (fbxDebug)
		DEBUG_LOG_FORMATTED("%s T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", str, tr[0], tr[1], tr[2], rot[0], rot[1], rot[2], sc[0], sc[1], sc[2]);
}
vector<IMesh*> ResourceManager::_FBXLoadMeshes(const char *pFullPath, const char *pRelativePath)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;
	vector<IMesh*> meshes;

	if (fbxDebug) LOG("Initializing FBX SDK...");

	_InitializeSdkObjects(lSdkManager, lScene);

	FbxString lFilePath(pFullPath);

	LOG_FORMATTED("Loading file: %s", lFilePath.Buffer());

	bool lResult = _LoadScene(lSdkManager, lScene, lFilePath.Buffer());

	if (!lResult)
		LOG_FATAL("An error occurred while loading the scene...");
	else
		_LoadSceneHierarchy(meshes, lScene, pFullPath, pRelativePath);

	if (fbxDebug) LOG("Destroying FBX SDK...");
	_DestroySdkObjects(lSdkManager, lResult);

	return std::move(meshes);
}
#endif


ResourceManager::ResourceManager()
{
	const char *pString = nullptr;
	_pCore->GetSubSystem((ISubSystem**)&_pFilesystem, SUBSYSTEM_TYPE::FILESYSTEM);

	_pCore->getConsoole()->addCommand("resources_list", std::bind(&ResourceManager::resources_list, this, std::placeholders::_1, std::placeholders::_2));
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Init()
{
	InitializeCriticalSection(&_cs);

	_pCore->GetSubSystem((ISubSystem**)&_pCoreRender, SUBSYSTEM_TYPE::CORE_RENDER);

	LOG("Resource Manager initalized");
}

API ResourceManager::resources_list(const char **args, uint argsNumber)
{
	LOG_FORMATTED("========= Runtime resources: %i =============", _runtime_textures.size() + _runtime_meshes.size() + _runtime_gameobjects.size() + _runtime_constntbuffer.size());

	LOG_FORMATTED("Runtime Meshes: %i", _runtime_meshes.size());
	LOG_FORMATTED("Runtime Textures: %i", _runtime_textures.size());
	LOG_FORMATTED("Runtime Game Objects: %i", _runtime_gameobjects.size());
	LOG_FORMATTED("Runtime Constnt Buffers: %i", _runtime_constntbuffer.size());
	LOG_FORMATTED("Runtime Shaders: %i", _runtime_shaders.size());

	LOG("Meshes:");
	for (auto it = _runtime_meshes.begin(); it != _runtime_meshes.end(); it++)
	{
		int refs;
		(*it)->GetReferences(&refs);
		LOG_FORMATTED("{refs = %i, type = %-25s, id = \"%s\"}", refs);
	}
	// TODO: other objects

	LOG_FORMATTED("========= Shared resources: %i =============", _shared_meshes.size() + _shared_textures.size() + _shared_shadertexts.size());
	LOG_FORMATTED("Shared Meshes: %i", _shared_meshes.size());
	LOG_FORMATTED("Shared Textures: %i", _shared_textures.size());
	LOG_FORMATTED("Shared Shader Texts: %i", _shared_shadertexts.size());

	LOG("Shared Meshes:");
	for (auto it = _shared_meshes.begin(); it != _shared_meshes.end(); it++)
	{
		IMesh *m = it->second;

		int refs;
		m->GetReferences(&refs);

		const char *path;
		m->GetFile(&path);

		LOG_FORMATTED("{refs = %i, file = \"%s\"}", refs, path);
	}
	// TODO: other objects

	return S_OK;
}

string ResourceManager::construct_full_path(const string& file)
{
	const char *pDataPath;
	_pCore->GetDataDir(&pDataPath);
	string dataPath = string(pDataPath);
	return dataPath + '\\' + file;
}

bool ResourceManager::error_if_path_not_exist(const string& fullPath)
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
	assert(_shared_meshes.size() == 0 && "ResourceManager::Free: _shared_meshes.size() != 0. You should release all meshes before free resource manager");
	assert(_runtime_meshes.size() == 0 && "ResourceManager::Free: _runtime_meshes.size() != 0. You should release all meshes before free resource manager");
	assert(_shared_textures.size() == 0 && "ResourceManager::Free: _shared_textures.size() != 0. You should release all textures before free resource manager");
	assert(_runtime_textures.size() == 0 && "ResourceManager::Free: _runtime_textures.size() != 0. You should release all textures before free resource manager");
	assert(_runtime_gameobjects.size() == 0 && "ResourceManager::Free: _runtime_gameobjects.size() != 0. You should release all gameobjects before free resource manager");

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

vector<IMesh*> ResourceManager::find_loaded_meshes(const char* pRelativeModelPath, const char *pMeshID)
{
	vector<IMesh*> out;

	for (auto it = _shared_meshes.begin(); it != _shared_meshes.end(); it++)
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

API ResourceManager::LoadModel(OUT IModel **pModel, const char *pModelPath)
{
	assert(is_relative(pModelPath) && "ResourceManager::LoadModel(): fileName must be relative");

	auto fullPath = construct_full_path(pModelPath);

	if (!error_if_path_not_exist(fullPath))
	{
		*pModel = nullptr;
		return E_FAIL;
	}

	vector<IMesh*> loaded_meshes = find_loaded_meshes(pModelPath, nullptr);

	IModel *model = nullptr;

	const string file_ext = ToLowerCase(fs::path(pModelPath).extension().string().erase(0, 1));

	if (loaded_meshes.size())
	{
		model = new Model(loaded_meshes);
		_runtime_gameobjects.emplace(model);
		DEBUG_LOG_FORMATTED("ResourceManager::LoadModel() new Model %#010x", model);
	} else	

#ifdef USE_FBX
	if (file_ext == "fbx")
	{
		loaded_meshes = _FBXLoadMeshes(fullPath.c_str(), pModelPath);
		for (IMesh *m : loaded_meshes)
		{
			const char *meshName;
			m->GetFile(&meshName);

			_shared_meshes.emplace(meshName, m);
		}
		model = new Model(loaded_meshes);
		_runtime_gameobjects.emplace(model);
		DEBUG_LOG_FORMATTED("ResourceManager::LoadModel() new Model %#010x", model);
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

API ResourceManager::LoadMesh(OUT IMesh **pMesh, const char *pMeshPath)
{
	string relativeModelPath;
	string meshID;
	split_mesh_path(pMeshPath, relativeModelPath, meshID);

	vector<IMesh*> loaded_meshes = find_loaded_meshes(relativeModelPath.c_str(), meshID.c_str());

	if (loaded_meshes.size())
	{
		*pMesh = loaded_meshes[0];
		return S_OK;
	}

	// check if standard mesh
	// then create new one

	ICoreMesh *stdCoreMesh = nullptr;

	if (!strcmp(pMeshPath, "std#plane"))
	{
		float vertexPlane[16] =
		{
			-1.0f, 1.0f, 0.0f, 1.0f,
			 1.0f,-1.0f, 0.0f, 1.0f,
			 1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 1.0f
		};

		unsigned short indexPlane[6]
		{
			0, 1, 2,
			0, 2, 3
		};

		MeshDataDesc desc;
		desc.pData = reinterpret_cast<uint8*>(vertexPlane);
		desc.numberOfVertex = 4;
		desc.positionStride = 16;

		MeshIndexDesc indexDesc;
		indexDesc.pData = reinterpret_cast<uint8*>(indexPlane);
		indexDesc.number = 6;
		indexDesc.format = MESH_INDEX_FORMAT::INT16;

		if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES)))
			return E_ABORT;

	} else if (!strcmp(pMeshPath, "std#axes"))
	{
		float vertexAxes[] = {0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,
								0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,
								0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f};

		MeshIndexDesc indexEmpty;

		MeshDataDesc descAxes;
		descAxes.pData = reinterpret_cast<uint8*>(vertexAxes);
		descAxes.numberOfVertex = 6;
		descAxes.positionStride = 32;
		descAxes.colorPresented = true;
		descAxes.colorOffset = 16;
		descAxes.colorStride = 32;

		if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES)))
			return E_ABORT;

	} else if (!strcmp(pMeshPath, "std#axes_arrows"))
	{
		// Layout: position, color, position, color, ...
		const float arrowRadius = 0.065f;
		const float arrowLength = 0.3f;
		const int segments = 12;
		const int numberOfVeretex = 3 * 3 * segments;
		const int floats = (4 + 4) * numberOfVeretex;

		float vertexAxesArrows[floats];
		void *M = vertexAxesArrows;
		for (int i = 0; i < 3; i++) // 3 axes
		{
			vec4 color;
			color.xyzw[i] = 1.0f;
			color.w = 1.0f;
			for (int j = 0; j < segments; j++)
			{
				constexpr float pi2 = 3.141592654f * 2.0f;
				float alpha = pi2 * (float(j) / segments);
				float dAlpha = pi2 * (1.0f / segments);

				vec4 v1, v2, v3;

				v1.xyzw[i] = 1.0f + arrowLength;
				v1.w = 1.0f;

				v2.xyzw[i] = 1.0f;
				v2.xyzw[(i + 1) % 3] = cos(alpha) * arrowRadius;
				v2.xyzw[(i + 2) % 3] = sin(alpha) * arrowRadius;
				v2.w = 1.0f;

				v3.xyzw[i] = 1.0f;
				v3.xyzw[(i + 1) % 3] = cos(alpha + dAlpha) * arrowRadius;
				v3.xyzw[(i + 2) % 3] = sin(alpha + dAlpha) * arrowRadius;
				v3.w = 1.0f;

				memcpy(vertexAxesArrows + i * segments * 24 + j * 24 + 0, &v1.x, 16);
				memcpy(vertexAxesArrows + i * segments * 24 + j * 24 + 4, &color.x, 16);
				memcpy(vertexAxesArrows + i * segments * 24 + j * 24 + 8, &v2.x, 16);
				memcpy(vertexAxesArrows + i * segments * 24 + j * 24 + 12, &color.x, 16);
				memcpy(vertexAxesArrows + i * segments * 24 + j * 24 + 16, &v3.x, 16);
				memcpy(vertexAxesArrows + i * segments * 24 + j * 24 + 20, &color.x, 16);
			}
		}
		MeshIndexDesc indexEmpty;

		MeshDataDesc descArrows;
		descArrows.pData = reinterpret_cast<uint8*>(vertexAxesArrows);
		descArrows.numberOfVertex = numberOfVeretex;
		descArrows.positionStride = 32;
		descArrows.colorPresented = true;
		descArrows.colorOffset = 16;
		descArrows.colorStride = 32;

		if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descArrows, &indexEmpty, VERTEX_TOPOLOGY::TRIANGLES)))
			return E_ABORT;

	} else if (!strcmp(pMeshPath, "std#grid"))
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

		if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&stdCoreMesh, &descGrid, &indexEmpty, VERTEX_TOPOLOGY::LINES)))
			return E_ABORT;
	}

	if (stdCoreMesh)
	{
		Mesh *m = new Mesh(stdCoreMesh, 1, pMeshPath);
		DEBUG_LOG_FORMATTED("ResourceManager::LoadMesh() new Mesh %#010x", m);
		*pMesh = m;
		_shared_meshes.emplace(pMeshPath, m);
		return S_OK;
	}

	#ifdef USE_FBX

		assert(false); // not impl

		//auto fullPath = constructFullPath(relativeModelPath);

		//if (!error_if_path_not_exist(fullPath))
		//{
		//	*pMesh = nullptr;
		//	return E_FAIL;
		//}

		//const string file_ext = ToLowerCase(fs::path(relativeModelPath).extension().string().erase(0, 1));
		//if (file_ext == "fbx")
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
		//	LOG_FATAL_FORMATTED("ResourceManager::LoadMesh unsupported format \"%s\"", file_ext.c_str());
		//	return E_FAIL;
		//}

	return S_OK;
}

API ResourceManager::LoadShaderText(OUT IShaderText **pShader, const char *pVertName, const char *pGeomName, const char *pFragName)
{
	auto load_shader = [=](string &paths, const char *&textOut, const char *pName) -> APIRESULT
	{
		IFile *pFile = nullptr;
		uint fileSize = 0;
		
		const char *pString;
		_pCore->GetInstalledDir(&pString);
		string installedDir = string(pString);

		string shader_path = installedDir + '\\' + SHADER_DIR + '\\' + pName + ".shader";

		if (!error_if_path_not_exist(shader_path))
			return S_FALSE;

		paths += shader_path + ";";

		_pFilesystem->OpenFile(&pFile, shader_path.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

		pFile->FileSize(&fileSize);

		char *tmp = new char[fileSize + 1];
		tmp[fileSize] = '\0';

		pFile->Read((uint8 *)tmp, fileSize);
		pFile->CloseAndFree();
				
		textOut = tmp;
		
		return S_OK;
	};

	string paths;
	const char *v = nullptr;
	const char *g = nullptr;
	const char *f = nullptr;

	auto ret =	load_shader(paths, v, pVertName);

	if (pGeomName)
		ret &=	load_shader(paths, g, pGeomName);

	ret &=		load_shader(paths, f, pFragName);

	DEBUG_LOG_FORMATTED("ResourceManager::LoadShaderText() new ShaderText");

	ShaderText *text = new ShaderText(v, g, f, paths);
	_shared_shadertexts.emplace(paths, text);
	*pShader = text;

	return ret;
}

API ResourceManager::LoadTexture(OUT ITexture **pTexture, const char *pMeshPath, TEXTURE_CREATE_FLAGS flags)
{
	*pTexture = nullptr;
	return E_NOTIMPL;
}

API ResourceManager::CreateGameObject(OUT IGameObject **pGameObject)
{
	DEBUG_LOG_FORMATTED("ResourceManager::CreateGameObject() new GameObject");

	IGameObject *g = new GameObject;

	_runtime_gameobjects.emplace(g);
	*pGameObject = g;

	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<IGameObject*>(g));

	return S_OK;
}

API ResourceManager::CreateModel(OUT IModel **pModel)
{
	DEBUG_LOG_FORMATTED("ResourceManager::CreateModel() new Model");

	IModel *g = new Model;

	_runtime_gameobjects.emplace(g);
	*pModel = g;

	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<IModel*>(g));

	return S_OK;
}

API ResourceManager::CreateCamera(OUT ICamera **pCamera)
{
	DEBUG_LOG_FORMATTED("ResourceManager::CreateCamera() new Camera");

	ICamera *g = new Camera;

	_runtime_gameobjects.emplace(g);
	*pCamera = g;

	SceneManager *sm = static_cast<SceneManager*>(getSceneManager(_pCore));
	sm->addGameObject(static_cast<ICamera*>(g));

	return S_OK;
}

API ResourceManager::CreateShader(OUT IShader **pShaderOut, const char *vert, const char *geom, const char *frag)
{
	ICoreShader *coreShader = nullptr;

	bool compiled = SUCCEEDED(_pCoreRender->CreateShader(&coreShader, vert, frag, geom)) && coreShader != nullptr;

	if (!compiled)
	{
		*pShaderOut = nullptr;
		LOG_WARNING("ResourceManager::CreateShader(): failed to create shader");
		return E_FAIL;
	}

	DEBUG_LOG_FORMATTED("ResourceManager::CreateShader() new Shader");

	IShader *s = new Shader(coreShader, vert, geom, frag);

	_runtime_shaders.emplace(s);

	*pShaderOut = s;

	return S_OK;
}

API ResourceManager::CreateConstantBuffer(OUT IConstantBuffer **pConstantBuffer, uint size)
{
	ICoreConstantBuffer *coreConstntBuffer = nullptr;

	bool created = SUCCEEDED(_pCoreRender->CreateConstantBuffer(&coreConstntBuffer, size)) && coreConstntBuffer != nullptr;

	if (!created)
	{
		*pConstantBuffer = nullptr;
		LOG_WARNING("ResourceManager::CreateConstantBuffer(): failed to create constnt buffer");
		return E_FAIL;
	}

	IConstantBuffer *cb = new ConstantBuffer(coreConstntBuffer);
	DEBUG_LOG_FORMATTED("ResourceManager::CreateConstantBuffer() new ConstantBuffer %#010x", cb);

	_runtime_constntbuffer.emplace(cb);

	*pConstantBuffer = cb;

	return S_OK;
}

