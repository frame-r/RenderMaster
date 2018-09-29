#include "pch.h"
#include "ResourceManager.h"
#include "Filesystem.h"
#include "Core.h"
#include "Model.h"
#include "Camera.h"
#include "Console.h"


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
	//int lFileFormat = -1;
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

void ResourceManager::_LoadSceneHierarchy(IModel *&pModel, FbxScene * pScene)
{
	vector<TResource<ICoreMesh> *> meshes;
	FbxString lString;

	if (fbxDebug) LOG("Scene hierarchy:");

	FbxNode* lRootNode = pScene->GetRootNode();
	_LoadNode(meshes, lRootNode, 0);

	//_LogNodeTransform(lRootNode, ("(root node!) " + lString + lRootNode->GetName()).Buffer());
	//for (int i = 0; i < lRootNode->GetChildCount(); i++)
	//	_LogNode(meshes, lRootNode->GetChild(i), 0);

	if (meshes.size() == 0)
		LOG_WARNING("No meshes loaded");

	pModel = new Model(meshes);

	for (TResource<ICoreMesh> *m : meshes)
		_resources.emplace(m);
}

void ResourceManager::_LoadNode(vector<TResource<ICoreMesh> *>& meshes, FbxNode* pNode, int depth)
{
	FbxString lString;
	FbxNodeAttribute* node = pNode->GetNodeAttribute();
	FbxNodeAttribute::EType lAttributeType = FbxNodeAttribute::eUnknown;
	if (node) lAttributeType = node->GetAttributeType();

	switch (lAttributeType)
	{
		case FbxNodeAttribute::eMesh:		_LoadMesh(meshes, (FbxMesh*)pNode->GetNodeAttribute(), pNode); break;
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
			_LoadNode(meshes, pNode->GetChild(i), depth + 1);
	}
}

void add_tabs(FbxString& buff, int tabs)
{
	for (int i = 0; i < tabs; i++)
		buff += " ";
}

void ResourceManager::_LoadMesh(vector<TResource<ICoreMesh> *>& meshes, FbxMesh *pMesh, FbxNode *pNode)
{
	int control_points_count = pMesh->GetControlPointsCount();
	int polygon_count = pMesh->GetPolygonCount();
	int normal_element_count = pMesh->GetElementNormalCount();
	int uv_layer_count = pMesh->GetElementUVCount();
	int tangent_layers_count = pMesh->GetElementTangentCount();
	int binormal_layers_count = pMesh->GetElementBinormalCount();

	string meshName = pMesh->GetName();
	string decorativeName = meshName + "::" + string(pNode->GetName());

	//FbxDouble3 tr = pNode->LclTranslation.Get();
	//FbxDouble3 rot = pNode->LclRotation.Get();
	//FbxDouble3 sc = pNode->LclScaling.Get();
	FbxVector4 tr = pNode->EvaluateGlobalTransform().GetT();
	FbxVector4 rot = pNode->EvaluateGlobalTransform().GetR();
	FbxVector4 sc = pNode->EvaluateGlobalTransform().GetS();
	//LOG_FORMATTED("T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2]);

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

	ICoreMesh *pCoreMesh = nullptr;
	
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

	_pCoreRender->CreateMesh((ICoreMesh**)&pCoreMesh, &vertDesc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);

	if (pCoreMesh)
		meshes.push_back(new TResource<ICoreMesh>(pCoreMesh, RES_TYPE::CORE_MESH, decorativeName));
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
bool ResourceManager::_FBXLoad(IModel *&pModel, const char *pFileName)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;

	if (fbxDebug) LOG("Initializing FBX SDK...");

	_InitializeSdkObjects(lSdkManager, lScene);

	FbxString lFilePath(pFileName);

	LOG_FORMATTED("Loading file: %s", lFilePath.Buffer());

	bool lResult = _LoadScene(lSdkManager, lScene, lFilePath.Buffer());

	if (!lResult)
		LOG_FATAL("An error occurred while loading the scene...");
	else
	{
		_LoadSceneHierarchy(pModel, lScene);
		//_ImportScene(lScene);
	}

	if (fbxDebug) LOG("Destroying FBX SDK...");
	_DestroySdkObjects(lSdkManager, lResult);

	//if (!lResult) fail_descr = ELF_FAIL;
	//else
	//{
	//	fail_descr = ELF_SUCCESS;
	//	vertex_data = &_vertecies[0].position[0];
	//	vertex_count = _vertecies.size();
	//	if (_indicies.empty())
	//	{
	//		indicies_data = nullptr;
	//		index_count = 0;
	//	}
	//	else
	//	{
	//		indicies_data = &_indicies[0];
	//		index_count = _indicies.size();
	//	}
	//}

	return true;
}
#endif

const char* ResourceManager::_resourceToStr(IResource *pRes)
{
	RES_TYPE type;
	pRes->GetType(&type);

	switch (type)
	{
		case RENDER_MASTER::RES_TYPE::CORE_MESH:
			return "CORE_MESH";
		case RENDER_MASTER::RES_TYPE::UNIFORM_BUFFER:
			return "UNIFORM_BUFFER";
		case RENDER_MASTER::RES_TYPE::GAME_OBJECT:
			return "GAMEOBJECT";
		case RENDER_MASTER::RES_TYPE::MODEL:
			return "MODEL";
		case RENDER_MASTER::RES_TYPE::CAMERA:
			return "CAMERA";
		case RENDER_MASTER::RES_TYPE::MESH_AXES:
			return "MESH_AXES";
		case RENDER_MASTER::RES_TYPE::MESH_AXES_ARROWS:
			return "MESH_AXES_ARROWS";
		case RENDER_MASTER::RES_TYPE::MESH_GRID:
			return "MESH_GRID";
		case RENDER_MASTER::RES_TYPE::MESH_PLANE:
			return "MESH_PLANE";
		default:
			return "UNKNOWN";
	}
	return nullptr;
}

ResourceManager::ResourceManager()
{
	const char *pString = nullptr;
	_pCore->GetSubSystem((ISubSystem**)&_pFilesystem, SUBSYSTEM_TYPE::FILESYSTEM);

	_pCore->getConsoole()->addCommand("resources_list", std::bind(&ResourceManager::_resources_list, this, std::placeholders::_1, std::placeholders::_2));
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

API ResourceManager::_resources_list(const char **args, uint argsNumber)
{
	LOG_FORMATTED("resources: %i", _resources.size());

	for (auto it = _resources.begin(); it != _resources.end(); it++)
	{
		IResource *res = *it;

		uint refs = 0;
		res->RefCount(&refs);

		const char *name;
		res->GetName(&name);

		LOG_FORMATTED("{refs = %i, type = %-25s,  name = %-30s}", refs, _resourceToStr(res), name);
	}

	return S_OK;
}

API ResourceManager::GetName(OUT const char **pName)
{
	*pName = "ResourceManager";
	return S_OK;
}

API ResourceManager::Free()
{
	DEBUG_LOG("ResourceManager::FreeAllResources(): resources total=%i", LOG_TYPE::NORMAL, _resources.size());

	// first free all resources that have refCount = 1
	// and so on...
	while (!_resources.empty())
	{
		vector<IResource*> one_ref_res;

		for (auto& res : _resources)
		{
			uint refs = 0;
			res->RefCount(&refs);
			if (refs <= 1)
				one_ref_res.push_back(res);
		}

#ifdef _DEBUG
		static int i = 0;
		i++;
		if (i > 20)
			return S_FALSE; // occured some error. maybe circular references => in debug limit number of iterations
		auto res_before = _resources.size();
#endif

		// free resources
		for (auto* pRes : one_ref_res)
		{
			auto it = _resources.find(pRes);
			if (it != _resources.end())
			{
				pRes->Release();
				_resources.erase(pRes);
			}
		}

#ifdef _DEBUG
		auto res_deleted = res_before - _resources.size();
		int res_deleted_percent = (int)(100 * ((float)res_deleted / res_before));
		DEBUG_LOG("ResourceManager::FreeAllResources(): (iteration=%i) : to delete=%i  deleted=%i (%i%%) resources left=%i", LOG_TYPE::NORMAL, i, one_ref_res.size(), res_deleted, res_deleted_percent, _resources.size());
#endif
	}

	_resources.clear();

	return S_OK;
}

API ResourceManager::LoadModel(OUT IResource **pModel, const char *pFileName)
{
	const string file_ext = ToLowerCase(fs::path(pFileName).extension().string().erase(0, 1));
	
	const char *pDataPath;
	_pCore->GetDataDir(&pDataPath);
	string dataPath = string(pDataPath);

	string fullPath = dataPath + '\\' + string(pFileName);

	IModel *model{nullptr};

#ifdef USE_FBX
	if (file_ext == "fbx")
	{
		bool ret = _FBXLoad(model, fullPath.c_str());
		if (!ret)
			return E_FAIL;
	}
	else
#endif
	{
		LOG_FATAL_FORMATTED("ResourceManager::LoadModel unsupported format \"%s\"", file_ext.c_str());
		return E_FAIL;
	}
		
	ISceneManager *pSceneManager;
	_pCore->GetSubSystem((ISubSystem**)&pSceneManager, SUBSYSTEM_TYPE::SCENE_MANAGER);

	TResource<IModel> *res = new TResource<IModel>(model, RES_TYPE::MODEL, string(pFileName));
	_resources.emplace(res);

	pSceneManager->AddRootGameObject(res);
	
	*pModel = res;

	return S_OK;
}

API ResourceManager::LoadShaderText(OUT IResource **pShader, const char *pVertName, const char *pGeomName, const char *pFragName)
{
	auto load_shader = [=](const char *&textOut, const char *pName) -> APIRESULT
	{
		IFile *pFile = nullptr;
		uint fileSize = 0;
		int filseExist = 0;
		
		const char *pString;
		_pCore->GetInstalledDir(&pString);
		string installedDir = string(pString);

		string shader_path = installedDir + '\\' + SHADER_DIR + '\\' + pName + ".shader";

		_pFilesystem->FileExist(const_cast<char*>(shader_path.c_str()), &filseExist);

		if (!filseExist)
		{
			LOG_WARNING_FORMATTED("ResourceManager::LoadShaderText(): File doesn't exist '%s'", shader_path.c_str());
			return S_FALSE;
		}

		_pFilesystem->OpenFile(&pFile, shader_path.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);

		pFile->FileSize(&fileSize);

		char *tmp = new char[fileSize + 1];
		tmp[fileSize] = '\0';

		pFile->Read((uint8 *)tmp, fileSize);
		pFile->CloseAndFree();
				
		textOut = tmp;
		
		return S_OK;
	};

	string fullName = string(pVertName);
	TResource<ShaderText> *res = new TResource<ShaderText>(new ShaderText, RES_TYPE::SHADER, fullName);
	_resources.emplace(res);

	auto ret =	load_shader((*res)->pVertText, pVertName);

	if (pGeomName)
		ret &=	load_shader((*res)->pGeomText, pGeomName);

	ret &=		load_shader((*res)->pFragText, pFragName);

	*pShader = res;

	return ret;
}

API ResourceManager::CreateResource(OUT IResource **pResource, RES_TYPE type)
{
	if (type > RES_TYPE::NUMBER)
	{
		*pResource = nullptr;
		LOG_WARNING("ResourceManager::GetDefaultResource(): unknown type of resource");
		return E_ABORT;
	}

	switch (type)
	{
	case RES_TYPE::MESH_PLANE:
	case RES_TYPE::MESH_AXES:
	case RES_TYPE::MESH_AXES_ARROWS:
	case RES_TYPE::MESH_GRID:
		{
			IResource *res = nullptr;

			auto it = std::find_if(_resources.begin(), _resources.end(), [type](IResource *res) -> bool
			{
				RES_TYPE next_type;

				res->GetType(&next_type);

				return next_type == type; 
			});

			if (it != _resources.end())
			{
				(*it)->AddRef();
				*pResource = *it;
			}

			// else create new resource

			ICoreMesh *ret = nullptr;

			if (type == RES_TYPE::MESH_PLANE)
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

				if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&ret, &desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES)))
					return E_ABORT;

			} else if (type == RES_TYPE::MESH_AXES)
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

				if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&ret, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES)))
					return E_ABORT;

			} else if (type == RES_TYPE::MESH_AXES_ARROWS)
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

				if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&ret, &descArrows, &indexEmpty, VERTEX_TOPOLOGY::TRIANGLES)))
					return E_ABORT;

			} else if (type == RES_TYPE::MESH_GRID)
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

				if (FAILED(_pCoreRender->CreateMesh((ICoreMesh**)&ret, &descGrid, &indexEmpty, VERTEX_TOPOLOGY::LINES)))
					return E_ABORT;
			}

			*pResource = new TResource<ICoreMesh>(ret, type, "CoreMesh");
			_resources.emplace(*pResource);
		}
		break;

		case RES_TYPE::CAMERA:
			*pResource = new TResource<ICamera>(new Camera, RES_TYPE::CAMERA, "Camera");
			_resources.emplace(*pResource);
			break;

		case RES_TYPE::GAME_OBJECT:
			*pResource = new TResource<IGameObject>(new GameObject, RES_TYPE::GAME_OBJECT, "GameObject");
			_resources.emplace(*pResource);
			break;
	}

	return S_OK;
}

API ResourceManager::CreateUniformBuffer(OUT IResource ** pResource, uint size)
{
	IUniformBuffer *buf = nullptr;
	_pCoreRender->CreateUniformBuffer(&buf, size);
	*pResource = new TResource<IUniformBuffer>(buf, RES_TYPE::UNIFORM_BUFFER, string("UniformBuffer: ") + to_string(size) + "b");
	_resources.emplace(*pResource);
	return S_OK;
}

API ResourceManager::GetNumberOfResources(OUT uint *number)
{
	*number = (uint) _resources.size();
	return S_OK;
}

API ResourceManager::DeleteResource(IResource *pResource)
{
	auto it = _resources.find(pResource);
	
	if (it == _resources.end())
	{
		LOG_WARNING("ResourceManager::DeleteResource() Resource not found\n");
		return E_ABORT;
	}

	uint refs = 0;
	(*it)->RefCount(&refs);

	if (refs != 0)
	{
		LOG_WARNING_FORMATTED("ResourceManager::DeleteResource() Unable delete resource: refs = %i\n", refs);
		return E_ABORT;
	}

	_resources.erase(pResource);

	return S_OK;
}
