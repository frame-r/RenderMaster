#include "ResourceManager.h"
#include "Core.h"
#include "Model.h"

#include <filesystem>
#include <cassert>
#include <iterator>

using namespace std;

#if defined _MSC_VER && _MSC_VER <= 1900
// VS 2015 ships filesystem as TS
namespace fs = std::experimental::filesystem;
#else
// for fully C++17 conformant toolchain
namespace fs = std::filesystem;
#endif

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
		LOG_FATAL("Error: Unable to create FBX Manager!");
		return;
	}
	else
		LOG_NORMAL_FORMATTED("Autodesk FBX SDK version %s", pManager->GetVersion());

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
		LOG("FBX SDK destroyed");
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

	LOG_FORMATTED("FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		LOG_FORMATTED("FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.
		lAnimStackCount = lImporter->GetAnimStackCount();

		LOG("Animation Stack Information:");
		LOG_FORMATTED("Number of Animation Stacks: %d", lAnimStackCount);
		LOG_FORMATTED("Current Animation Stack: \"%s\"", lImporter->GetActiveAnimStackName().Buffer());

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

void ResourceManager::_LogSceneHierarchy(FbxScene * pScene)
{
	LOG("Scene hierarchy:");

	FbxNode* lRootNode = pScene->GetRootNode();

	for (int i = 0; i < lRootNode->GetChildCount(); i++)
		_LogNode(lRootNode->GetChild(i), 0);
}

void ResourceManager::_LogNode(FbxNode* pNode, int depth)
{
	FbxString lString;

	for (int i = 0; i < depth; i++)
		lString += " ";

	lString += pNode->GetName();

	FbxNodeAttribute::EType lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

	switch (lAttributeType)
	{
	case FbxNodeAttribute::eMesh:		lString += " (eMesh)"; LOG(lString.Buffer()); _LogNodeTransform(pNode, depth + 3); _LogMesh((FbxMesh*)pNode->GetNodeAttribute(), depth + 3); break;
		case FbxNodeAttribute::eMarker:		lString += " (eMarker)"; LOG(lString.Buffer()); break;
		case FbxNodeAttribute::eSkeleton:	lString += " (eSkeleton)"; LOG(lString.Buffer()); break;
		case FbxNodeAttribute::eNurbs:		lString += " (eNurbs)"; LOG(lString.Buffer()); break;
		case FbxNodeAttribute::ePatch:		lString += " (ePatch)"; LOG(lString.Buffer()); break;
		case FbxNodeAttribute::eCamera:		lString += " (eCamera)"; LOG(lString.Buffer()); _LogNodeTransform(pNode, depth + 1); break;
		case FbxNodeAttribute::eLight:		lString += " (eLight)"; LOG(lString.Buffer()); break;
		case FbxNodeAttribute::eLODGroup:	lString += " (eLODGroup)"; LOG(lString.Buffer()); break;
		default:							lString += " (unknown)"; LOG(lString.Buffer()); break;
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
		_LogNode(pNode->GetChild(i), depth + 1);
}

void add_tabs(FbxString& buff, int tabs)
{
	for (int i = 0; i < tabs; i++)
		buff += " ";
}

void ResourceManager::_LogMesh(FbxMesh *pMesh, int tabs)
{
	int cp = pMesh->GetControlPointsCount();
	int pc = pMesh->GetPolygonCount();
	int normal_layers = pMesh->GetElementNormalCount();
	int uv_layers = pMesh->GetElementUVCount();
	int tangent_layers = pMesh->GetElementTangentCount();
	int binormal_layers = pMesh->GetElementBinormalCount();

	LOG_FORMATTED("ControlPoints=%d PolygonCount=%d NormalLayers=%d UVLayers=%d TangentLayers=%d BinormalLayers=%d\r\n", cp, pc, normal_layers, uv_layers, tangent_layers, binormal_layers);

	for (int i = 0; i< normal_layers; i++)
	{
		FbxString buff;
		FbxGeometryElementNormal* ns = pMesh->GetElementNormal(i);

		LOG_FORMATTED("Normals=%d", i);		

		switch (ns->GetMappingMode())
		{
			case FbxLayerElement::eByControlPoint:	buff += "EMappingMode=eByControlPoint\r\n"; break;
			case FbxLayerElement::eByPolygonVertex:	buff += "EMappingMode=eByPolygonVertex\r\n"; break;
			case FbxLayerElement::eNone:			buff += "EMappingMode=eNone\r\n"; break;
			case FbxLayerElement::eByPolygon:		buff += "EMappingMode=eByPolygon\r\n"; break;
			case FbxLayerElement::eByEdge:			buff += "EMappingMode=eByEdge\r\n"; break;
			case FbxLayerElement::eAllSame:			buff += "EMappingMode=eAllSame\r\n"; break;
			default:								buff += "EMappingMode=unknown\r\n"; break;;
		}

		switch (ns->GetReferenceMode())
		{
			case FbxLayerElement::eDirect:			buff += " EReferenceMode=eDirect "; break;
			case FbxLayerElement::eIndex:			buff += " EReferenceMode=eIndex "; break;
			case FbxLayerElement::eIndexToDirect:	buff += " EReferenceMode=eIndexToDirect "; break;
			default:								buff += " EReferenceMode=unknown "; break;;
		}

		LOG(buff.Buffer());
	}	
}

void ResourceManager::_LogNodeTransform(FbxNode* pNode, int tabs)
{
	FbxVector4 lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

	LOG_FORMATTED("Translation: %f %f %f  Rotation: %f %f %f  Scaling: %f %f %f", lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2]);
}
bool ResourceManager::_FBXLoad(IModel *&pModel, const char *pFileName, IProgressSubscriber *pPregress)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;

	LOG("Initializing FBX SDK...");

	_InitializeSdkObjects(lSdkManager, lScene);

	FbxString lFilePath(pFileName);

	LOG_FORMATTED("Loading file: %s", lFilePath.Buffer());

	bool lResult = _LoadScene(lSdkManager, lScene, lFilePath.Buffer());

	if (!lResult)
		LOG_FATAL("An error occurred while loading the scene...");
	else
	{
		_LogSceneHierarchy(lScene);
		//_ImportScene(lScene);
	}

	LOG("Destroying FBX SDK...");
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

	return false;
}
const char * ResourceManager::resourceToStr(IResource * pRes)
{
	RES_TYPE type;
	pRes->GetType(type);

	switch (type)
	{
	case RENDER_MASTER::RES_TYPE::RT_CORE_MESH:
		return "RT_CORE_MESH";
	case RENDER_MASTER::RES_TYPE::RT_CORE_TEXTURE:
		return "RT_CORE_TEXTURE";
	case RENDER_MASTER::RES_TYPE::RT_CORE_SHADER:
		return "RT_CORE_SHADER";
	case RENDER_MASTER::RES_TYPE::RT_GAMEOBJECT:
		return "RT_GAMEOBJECT";
	case RENDER_MASTER::RES_TYPE::RT_MODEL:
		return "RT_MODEL";
	}

	return nullptr;
}
#endif

ResourceManager::ResourceManager()
{		
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Init()
{
	_pCore->GetSubSystem((ISubSystem*&)_pCoreRender, SUBSYSTEM_TYPE::ST_CORE_RENDER);

	InitializeCriticalSection(&_cs);

	float vertex[12] = 
	{
		-1.0f, 1.0f, 0.0f,
		 1.0f,-1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 -1.0f, 1.0f, 0.0f
	};

	short index[6]
	{
		0, 1, 2,
		0, 2, 3
	};

	ICoreMesh *pPlane;
	
	MeshDataDesc desc;
	desc.pData = reinterpret_cast<uint8*>(vertex);
	desc.number = 4;

	MeshIndexDesc indexDesc;
	indexDesc.pData = reinterpret_cast<uint8*>(index);
	indexDesc.number = 6;
	indexDesc.format = MESH_INDEX_FORMAT::MID_INT16;

	_pCoreRender->CreateMesh((ICoreMesh*&)pPlane, desc, indexDesc, DRAW_MODE::DM_TRIANGLES);

	_default_meshes.emplace(DEFAULT_RESOURCE_TYPE::DRT_PLANE, pPlane);

	LOG("ResourceManager initalized");
}

API ResourceManager::GetName(const char *& pTxt)
{
	pTxt = "ResourceManager";
	return S_OK;
}

API ResourceManager::LoadModel(IModel *&pModel, const char *pFileName, IProgressSubscriber *pPregress)
{
	const string file_ext = ToLowerCase(fs::path(pFileName).extension().string().erase(0, 1));
	
	const char *pDataPath;
	_pCore->GetDataPath(pDataPath);
	string fullPath = string(pDataPath) + '\\' + string(pFileName);

#ifdef USE_FBX
	if (file_ext == "fbx")
	{
		bool ret = _FBXLoad(pModel, fullPath.c_str(), pPregress);
		if (!ret)
			return S_FALSE;
	}
	else
#endif
	{
		LOG_FATAL_FORMATTED("ResourceManager::LoadModel unsupported format \"%s\"", file_ext.c_str());
		return S_FALSE;
	}

	AddToList(pModel);

	uint meshNumber;
	pModel->GetMeshesNumber(meshNumber);

	for (uint i = 0; i < meshNumber; i++)
	{
		ICoreMesh *pMesh;

		pModel->GetMesh(pMesh, i);
		AddToList(pMesh);
	}
	
	return S_OK;
}

API ResourceManager::CreateDefaultModel(IModel *&pModel, DEFAULT_RESOURCE_TYPE type)
{
	auto it = _default_meshes.find(type);

	ICoreMesh *pCoreMesh = it->second;
	Model *_pModel = new Model(pCoreMesh);
	
	AddToList(pCoreMesh);
	AddToList(_pModel);

	pModel = _pModel;

	return S_OK;
}

API ResourceManager::LoadShader(ICoreShader *& pShader, const char * pVertName, const char * pGeomName, const char * pFragName)
{
	return E_NOTIMPL;
}

API ResourceManager::AddToList(IResource *pResource)
{ 
	auto it = std::find_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
	if (it == _res_vec.end())
	{
		_res_vec.push_back(TResource{ pResource, 1 });
	
		DEBUG_LOG("AddToList(): added new resource! type=%s", LOG_TYPE::LT_NORMAL, resourceToStr(pResource));
	}
	else
	{
		it->refCount++;
	
		DEBUG_LOG("AddToList(): refCount++ refCount==%i type=%s", LOG_TYPE::LT_NORMAL, it->refCount, resourceToStr(pResource));
	}

	return S_OK;
}

API ResourceManager::GetRefNumber(IResource *pResource, uint& number)
{
	auto it = std::find_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });

	if (it == _res_vec.end())
		return E_POINTER;

	assert(it->refCount > 0);

	number = it->refCount;

	return S_OK;
}

API ResourceManager::DecrementRef(IResource * pResource)
{
	auto it = std::find_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
	
	if (it == _res_vec.end())
		return E_POINTER;

	assert(it->refCount > 0);

	it->refCount--;

	DEBUG_LOG("DecrementRef(): refCount-- refCount==%i type=%s", LOG_TYPE::LT_NORMAL, it->refCount, resourceToStr(pResource));

	return S_OK;
}

API ResourceManager::RemoveFromList(IResource *pResource)
{
	uint refCount;
	GetRefNumber(pResource, refCount);

	if (refCount == 1)
	{
		auto it = std::remove_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
		_res_vec.erase(it, _res_vec.end());

		DEBUG_LOG("RemoveFromList(): deleted! type=%s", LOG_TYPE::LT_NORMAL, resourceToStr(pResource));
	}
	else
		LOG_WARNING_FORMATTED("RemoveFromList(): not deleted! refNumber=%i type=%s", refCount, resourceToStr(pResource));

	return S_OK;
}

API ResourceManager::FreeAllResources()
{
	DEBUG_LOG("FreeAllResources(): resorces total=%i", LOG_TYPE::LT_NORMAL, _res_vec.size());

	// first free all resources that have refCount = 1
	// and so on...
	while (!_res_vec.empty())
	{
		vector<TResource> one_ref_res;

		auto ref_is_1 = [](const TResource& res) {return res.refCount == 1; };

		// partition: 
		// all elements that should not be moved come before 
		// all elements that should be moved come after
		// stable_partition maintains relative order in each group
		auto p = std::stable_partition(_res_vec.begin(), _res_vec.end(), [&](const auto& x) { return !ref_is_1(x); });

		// move range elements that sould be removed
		one_ref_res.insert(one_ref_res.end(), std::make_move_iterator(p), std::make_move_iterator(_res_vec.end()));
		
		// erase the moved-from elements.
		// no need this because pRes->Free() removes himself from the vector!
		//_res_vec.erase(p, _res_vec.end());
		
		#ifdef _DEBUG
			static int i = 0;
			i++;
			if (i > 20) break; // occured some error. maybe circular references => in debug limit number of iterations
			auto res = _res_vec.size();
			DEBUG_LOG("FreeAllResources(): beginIteration=%i resourceToDelete=%i", LOG_TYPE::LT_NORMAL, i, one_ref_res.size(), _res_vec.size());
		#endif

		// free elements in group
		for (auto res : one_ref_res)
			res.pRes->Free();

		#ifdef _DEBUG
			auto deleted = res - _res_vec.size();
			DEBUG_LOG("FreeAllResources(): endIteration=%i resourcesDeleted=%i, resourcesLeft=%i", LOG_TYPE::LT_NORMAL, i, deleted, _res_vec.size());
		#endif
	}
	
	return S_OK;
}
