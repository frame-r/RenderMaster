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


#ifdef USE_FBX

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pManager->GetIOSettings()))
#endif

extern Core *_pCore;

void ResourceManager::_InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{

	pManager = FbxManager::Create();
	if (!pManager)
	{
		_pCore->Log("Error: Unable to create FBX Manager!", LOG_TYPE::LT_FATAL);
		exit(1);
	}
	else
	{
		_pCore->Log("Autodesk FBX SDK version ");
		_pCore->Log(pManager->GetVersion());
	}

	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");
	if (!pScene)
	{
		_pCore->Log("Error: Unable to create FBX scene!", LOG_TYPE::LT_FATAL);
		exit(1);
	}
}

void ResourceManager::_DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
	if (pManager) pManager->Destroy();
	if (pExitStatus) _pCore->Log("FBX SDK destroyed");
}

bool ResourceManager::_LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	//int lFileFormat = -1;
	int lAnimStackCount;
	bool lStatus;
	char lPassword[1024];

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
		_pCore->Log("Call to FbxImporter::Initialize() failed.", LOG_TYPE::LT_FATAL);
		sprintf_s(buffer, "Error returned: %s", error.Buffer());
		_pCore->Log(buffer);

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			sprintf_s(buffer, "FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);
			_pCore->Log(buffer);
			sprintf_s(buffer, "FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);
			_pCore->Log(buffer);
		}

		return false;
	}

	sprintf_s(buffer, "FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);
	_pCore->Log(buffer);

	if (lImporter->IsFBX())
	{
		sprintf_s(buffer, "FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);
		_pCore->Log(buffer);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		_pCore->Log("Animation Stack Information");

		lAnimStackCount = lImporter->GetAnimStackCount();

		sprintf_s(buffer, "Number of Animation Stacks: %d", lAnimStackCount);
		_pCore->Log(buffer);
		sprintf_s(buffer, "Current Animation Stack: \"%s\"", lImporter->GetActiveAnimStackName().Buffer());
		_pCore->Log(buffer);


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
	{
		_pCore->Log("Please enter password: ");

		lPassword[0] = '\0';

		FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
			scanf("%s", lPassword);
		FBXSDK_CRT_SECURE_NO_WARNING_END

			FbxString lString(lPassword);

		IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
		IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

		lStatus = lImporter->Import(pScene);

		if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			_pCore->Log("Password is wrong, import aborted.", LOG_TYPE::LT_FATAL);
		}
	}

	// Destroy the importer.
	lImporter->Destroy();

	return lStatus;
}

void ResourceManager::_LogSceneHierarchy(FbxScene * pScene)
{
	_pCore->Log("Scene hierarchy:");
	FbxNode* lRootNode = pScene->GetRootNode();

	for (int i = 0; i < lRootNode->GetChildCount(); i++)
	{
		_LogNode(lRootNode->GetChild(i), 0);
	}
}

void ResourceManager::_LogNode(FbxNode* pNode, int depth)
{
	FbxString lString;
	int i;

	for (i = 0; i < depth; i++)
	{
		lString += " ";
	}
	lString += pNode->GetName();

	FbxNodeAttribute::EType lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

	switch (lAttributeType)
	{
	case FbxNodeAttribute::eMesh:		lString += " (eMesh)"; _pCore->Log(lString.Buffer()); _LogNodeTransform(pNode, depth + 3); _LogMesh((FbxMesh*)pNode->GetNodeAttribute(), depth + 3); break;
	case FbxNodeAttribute::eMarker:		lString += " (eMarker)"; _pCore->Log(lString.Buffer()); break;
	case FbxNodeAttribute::eSkeleton:	lString += " (eSkeleton)"; _pCore->Log(lString.Buffer()); break;
	case FbxNodeAttribute::eNurbs:		lString += " (eNurbs)"; _pCore->Log(lString.Buffer()); break;
	case FbxNodeAttribute::ePatch:		lString += " (ePatch)"; _pCore->Log(lString.Buffer()); break;
	case FbxNodeAttribute::eCamera:		lString += " (eCamera)"; _pCore->Log(lString.Buffer()); _LogNodeTransform(pNode, depth + 1); break;
	case FbxNodeAttribute::eLight:		lString += " (eLight)"; _pCore->Log(lString.Buffer()); break;
	case FbxNodeAttribute::eLODGroup:	lString += " (eLODGroup)"; _pCore->Log(lString.Buffer()); break;
	default:							lString += " (unknown)"; _pCore->Log(lString.Buffer()); break;
	}

	for (i = 0; i < pNode->GetChildCount(); i++)
	{
		_LogNode(pNode->GetChild(i), depth + 1);
	}
}

void add_tabs(FbxString& buff, int tabs)
{
	for (int i = 0; i < tabs; i++)
		buff += " ";
}

void ResourceManager::_LogMesh(FbxMesh *pMesh, int tabs)
{
	FbxString buff;

	int cp = pMesh->GetControlPointsCount();
	int pc = pMesh->GetPolygonCount();
	int normal_layers = pMesh->GetElementNormalCount();
	int uv_layers = pMesh->GetElementUVCount();
	int tangent_layers = pMesh->GetElementTangentCount();
	int binormal_layers = pMesh->GetElementBinormalCount();

	add_tabs(buff, tabs);
	sprintf_s(buffer, "ControlPoints=%d PolygonCount=%d NormalLayers=%d UVLayers=%d TangentLayers=%d BinormalLayers=%d\r\n", cp, pc, normal_layers, uv_layers, tangent_layers, binormal_layers);
	buff += buffer;

	for (int i = 0; i< normal_layers; i++)
	{
		add_tabs(buff, tabs);
		sprintf_s(buffer, "Normals=%d\r\n", i);
		buff += buffer;

		FbxGeometryElementNormal* ns = pMesh->GetElementNormal(i);

		add_tabs(buff, tabs + 1);
		FbxGeometryElement::EMappingMode map_mode = ns->GetMappingMode();
		switch (map_mode)
		{
		case FbxLayerElement::eByControlPoint:	buff += "EMappingMode=eByControlPoint\r\n"; break;
		case FbxLayerElement::eByPolygonVertex:	buff += "EMappingMode=eByPolygonVertex\r\n"; break;
		case FbxLayerElement::eNone:			buff += "EMappingMode=eNone\r\n"; break;
		case FbxLayerElement::eByPolygon:		buff += "EMappingMode=eByPolygon\r\n"; break;
		case FbxLayerElement::eByEdge:			buff += "EMappingMode=eByEdge\r\n"; break;
		case FbxLayerElement::eAllSame:			buff += "EMappingMode=eAllSame\r\n"; break;
		default:								buff += "EMappingMode=unknown\r\n"; break;;
		}

		add_tabs(buff, tabs + 1);
		FbxGeometryElement::EReferenceMode ref_mode = ns->GetReferenceMode();
		switch (ref_mode)
		{
		case FbxLayerElement::eDirect:			buff += "EReferenceMode=eDirect "; break;
		case FbxLayerElement::eIndex:			buff += "EReferenceMode=eIndex "; break;
		case FbxLayerElement::eIndexToDirect:	buff += "EReferenceMode=eIndexToDirect "; break;
		default:								buff += "EReferenceMode=unknown "; break;;
		}
	}
	_pCore->Log(buff.Buffer());
}

void ResourceManager::_LogNodeTransform(FbxNode* pNode, int tabs)
{
	FbxString buff;

	FbxVector4 lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	
	add_tabs(buff, tabs);
	sprintf_s(buffer, "Translation: %f %f %f ", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
	buff += buffer;

	lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	add_tabs(buff, tabs);
	sprintf_s(buffer, "Rotation: %f %f %f ", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
	buff += buffer;

	lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	add_tabs(buff, tabs);
	sprintf_s(buffer, "Scaling: %f %f %f ", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
	buff += buffer;

	_pCore->Log(buff.Buffer());
}
bool ResourceManager::_FBXLoad(IModel *&pMesh, const char *pFileName, IProgressSubscriber *pPregress)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;

	_pCore->Log("Initializing FBX SDK...");
	_InitializeSdkObjects(lSdkManager, lScene);


	FbxString lFilePath(pFileName);

	_pCore->Log("Loading file: ");
	_pCore->Log(lFilePath.Buffer());

	bool lResult = _LoadScene(lSdkManager, lScene, lFilePath.Buffer());

	if (lResult == false)
		_pCore->Log("An error occurred while loading the scene...", LOG_TYPE::LT_FATAL);
	else
	{
		_LogSceneHierarchy(lScene);
		//_ImportScene(lScene);
	}

	_pCore->Log("Destroying FBX SDK...");
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
	case RENDER_MASTER::RES_TYPE::RT_REFERENCEBLE_END:
		return "RT_REFERENCEBLE_END";
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
	FreeAllResources();
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

	_default_meshes.emplace(DEFAULT_RESOURCE_TYPE::RT_PLANE, pPlane);

	_pCore->Log("ResourceManager initalized");
}

API ResourceManager::GetName(const char *& pTxt)
{
	pTxt = "ResourceManager";
	return S_OK;
}

API ResourceManager::LoadModel(IModel *&pModel, const char *pFileName, IProgressSubscriber *pPregress)
{
	const string file_ext = ToLowerCase(fs::path(pFileName).extension().string().erase(0, 1));
	
	IModel *_pModel;

#ifdef USE_FBX
	if (file_ext == "fbx")
	{
		bool ret = _FBXLoad(_pModel, pFileName, pPregress);
		if (!ret)
			return S_FALSE;
	}
	else
#endif
	{		
		_pCore->LogFormatted("ResourceManager::LoadModel unsupported format \"%s\"", LOG_TYPE::LT_FATAL, file_ext.c_str());
		
		return S_FALSE;
	}

	AddToList(_pModel);

	uint meshNumber;
	_pModel->GetMeshesNumber(meshNumber);

	for (uint i = 0; i < meshNumber; i++)
	{
		ICoreMesh *pMesh;

		_pModel->GetMesh(pMesh, i);
		AddToList(pMesh);
	}
	
	return S_OK;
}

API ResourceManager::CreateDefaultModel(IModel *&pModel, DEFAULT_RESOURCE_TYPE type)
{
	auto it = _default_meshes.find(type);

	assert(it != _default_meshes.end());

	ICoreMesh *pCoreMesh = it->second;
	Model *_pModel = new Model(pCoreMesh);

	AddToList(pCoreMesh);
	AddToList(_pModel);

	pModel = _pModel;

	return S_OK;
}

API ResourceManager::LoadShader(ICoreShader *& pShader, const char * pVertName, const char * pGeomName, const char * pFragName)
{
	return S_OK;
}

API ResourceManager::AddToList(IResource *pResource)
{ 
	auto it = std::find_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
	if (it != _res_vec.end())
	{
		it->refCount++;
#ifdef _DEBUG
		_pCore->LogFormatted("AddToList(): refCount++ refCount==%i type=%s", LOG_TYPE::LT_NORMAL, it->refCount, resourceToStr(pResource));
#endif
	}
	else
	{
		_res_vec.push_back(TResource{ pResource, 1 });

#ifdef _DEBUG
		_pCore->LogFormatted("AddToList(): refCount=1 type=%s", LOG_TYPE::LT_NORMAL, resourceToStr(pResource));
#endif
	}


	return S_OK;
}

API ResourceManager::GetRefNumber(IResource * pResource, uint& number)
{
	auto it = std::find_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
	if (it == _res_vec.end())
	{
		return E_POINTER;
	}
	assert(it->refCount > 0);
	number = it->refCount;
	return S_OK;
}

API ResourceManager::DecrementRef(IResource * pResource)
{
	auto it = std::find_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
	if (it == _res_vec.end())
	{
		return E_POINTER;
	}
	assert(it->refCount > 0);
	it->refCount--;

#ifdef _DEBUG
	_pCore->LogFormatted("DecrementRef(): refCount-- refCount==%i type=%s", LOG_TYPE::LT_NORMAL, it->refCount, resourceToStr(pResource));
#endif

	return S_OK;
}

API ResourceManager::RemoveFromList(IResource * pResource)
{
	uint number;
	GetRefNumber(pResource, number);

	if (number == 1)
	{
		auto it = std::remove_if(_res_vec.begin(), _res_vec.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
		_res_vec.erase(it, _res_vec.end());

#ifdef _DEBUG
		_pCore->LogFormatted("RemoveFromList(): deleted! type=%s", LOG_TYPE::LT_NORMAL, resourceToStr(pResource));
#endif
	}
	else
		_pCore->LogFormatted("RemoveFromList(): not deleted! refNumber=%i type=%s", LOG_TYPE::LT_WARNING, number, resourceToStr(pResource));

	return S_OK;
}

API ResourceManager::FreeAllResources()
{
#ifdef _DEBUG
	_pCore->LogFormatted("FreeAllResources(): total=%i", LOG_TYPE::LT_NORMAL, _res_vec.size());
#endif

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
		if (i > 20) break;
		auto res = _res_vec.size();
		_pCore->LogFormatted("FreeAllResources(): beginIteration=%i resourceToDelete=%i", LOG_TYPE::LT_NORMAL, i, one_ref_res.size(), _res_vec.size());
#endif

		// free elements in group
		for (auto res : one_ref_res)
			res.pRes->Free();

#ifdef _DEBUG
		auto deleted = res - _res_vec.size();
		_pCore->LogFormatted("FreeAllResources(): endIteration=%i resourcesDeleted=%i, resourcesLeft=%i", LOG_TYPE::LT_NORMAL, i, deleted, _res_vec.size());
#endif
	}
	
	return S_OK;
}
