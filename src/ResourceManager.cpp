#include "ResourceManager.h"

#include "Filesystem.h"
#include "Core.h"
#include "Model.h"

#include <filesystem>
#include <cassert>
#include <iterator>


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

#define SHADER_DIR "src\\shaders"

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
		LOG_FATAL("[FBX]Error: Unable to create FBX Manager!");
		return;
	}

	LOG_NORMAL_FORMATTED("[FBX]Autodesk FBX SDK version %s", pManager->GetVersion());

	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");

	if (!pScene)
	{
		LOG_FATAL("[FBX]Error: Unable to create FBX scene!");
		return;
	}
}

void ResourceManager::_DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
	if (pManager) pManager->Destroy();
	if (pExitStatus)
		LOG("[FBX]FBX SDK destroyed");
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

		LOG_FATAL("[FBX]Call to FbxImporter::Initialize() failed.");
		LOG_FATAL_FORMATTED("Error returned: %s", error.Buffer());

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			LOG_FORMATTED("[FBX]FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);
			LOG_FORMATTED("[FBX]FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	LOG_FORMATTED("[FBX]FBX file format version for this FBX SDK is %d.%d.%d", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		LOG_FORMATTED("[FBX]FBX file format version for file '%s' is %d.%d.%d", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.
		lAnimStackCount = lImporter->GetAnimStackCount();

		LOG("[FBX]Animation Stack Information:");
		LOG_FORMATTED("[FBX]Number of Animation Stacks: %d", lAnimStackCount);
		LOG_FORMATTED("[FBX]Current Animation Stack: \"%s\"", lImporter->GetActiveAnimStackName().Buffer());

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
		LOG_FATAL("[FBX]No support entering password!");

	lImporter->Destroy();

	return lStatus;
}

void ResourceManager::_LogSceneHierarchy(IModel *&pModel, FbxScene * pScene)
{
	vector<ICoreMesh *> meshes;

	LOG("[FBX]Scene hierarchy:");

	FbxNode* lRootNode = pScene->GetRootNode();

	for (int i = 0; i < lRootNode->GetChildCount(); i++)
		_LogNode(meshes, lRootNode->GetChild(i), 0);

	if (meshes.size() == 0)
		LOG_WARNING("[FBX]No meshes loaded");

	pModel = new Model(meshes);
}

void ResourceManager::_LogNode(vector<ICoreMesh *>& meshes, FbxNode* pNode, int depth)
{
	FbxString lString;
	FbxNodeAttribute::EType lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

	switch (lAttributeType)
	{
		case FbxNodeAttribute::eMesh:		_LogMesh(meshes, (FbxMesh*)pNode->GetNodeAttribute(), pNode); break;
		case FbxNodeAttribute::eMarker:		LOG(("[FBX] (eMarker) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eSkeleton:	LOG(("[FBX] (eSkeleton) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eNurbs:		LOG(("[FBX] (eNurbs) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::ePatch:		LOG(("[FBX] (ePatch) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eCamera:		_LogNodeTransform(pNode, depth + 1); break;
		case FbxNodeAttribute::eLight:		LOG(("[FBX] (eLight) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLODGroup:	LOG(("[FBX] (eLODGroup) " + lString + pNode->GetName()).Buffer()); break;
		default:							LOG(("[FBX] (UNKNOWN!) " + lString + pNode->GetName()).Buffer()); break;
	}

	for (int i = 0; i < pNode->GetChildCount(); i++)
		_LogNode(meshes, pNode->GetChild(i), depth + 1);
}

void add_tabs(FbxString& buff, int tabs)
{
	for (int i = 0; i < tabs; i++)
		buff += " ";
}

void ResourceManager::_LogMesh(vector<ICoreMesh *>& meshes, FbxMesh *pMesh, FbxNode *pNode)
{
	int control_points_count = pMesh->GetControlPointsCount();
	int polygon_count = pMesh->GetPolygonCount();
	int normal_element_count = pMesh->GetElementNormalCount();
	int uv_layer_count = pMesh->GetElementUVCount();
	int tangent_layers_count = pMesh->GetElementTangentCount();
	int binormal_layers_count = pMesh->GetElementBinormalCount();

	FbxVector4 lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	//LOG_FORMATTED("[FBX]T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2]);

	LOG_FORMATTED("[FBX] (eMesh) %10s T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f) CP=%5d POLYS=%5d NORMAL=%d UV=%d TANG=%d BINORM=%d", 
		pNode->GetName(),
		lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2],
		control_points_count, polygon_count, normal_element_count, uv_layer_count, tangent_layers_count, binormal_layers_count);

	struct Vertex
	{
		float x, y, z;
		float n_x, n_y, n_z;
	};

	vector<Vertex> vertecies;
	vertecies.reserve(polygon_count * 3);

	FbxVector4* p_control_points = pMesh->GetControlPoints();

	int global_vert_id = 0;

	for (int i = 0; i < polygon_count; i++)
	{
		int polygon_size = pMesh->GetPolygonSize(i);

		for (int t = 0; t < polygon_size - 2; t++) // triangulate large polygons
			for (int j = 0; j < 3; j++)
			{
				Vertex v;

				int vert_idx = t + j;
				if (j == 0)
					vert_idx = 0;

				int fbx_idx = pMesh->GetPolygonVertex(i, vert_idx);

				// position
				FbxVector4 lCurrentVertex = p_control_points[fbx_idx];
				v.x = (float)lCurrentVertex[0];
				v.y = (float)lCurrentVertex[1];
				v.z = (float)lCurrentVertex[2];

				// normal
				if (normal_element_count)
				{
					FbxGeometryElementNormal* pNormals = pMesh->GetElementNormal(0);
					FbxVector4 norm;

					switch (pNormals->GetMappingMode())
					{
						case FbxGeometryElement::eByControlPoint:
						{
							switch (pNormals->GetReferenceMode())
							{
								case FbxGeometryElement::eDirect:
									norm = pNormals->GetDirectArray().GetAt(i);
								break;
								case FbxGeometryElement::eIndexToDirect:
								{
									int id = pNormals->GetIndexArray().GetAt(i);
									norm = pNormals->GetDirectArray().GetAt(id);
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
								norm = pNormals->GetDirectArray().GetAt(global_vert_id + vert_idx);
								break;
							case FbxGeometryElement::eIndexToDirect:
							{
								int id = pNormals->GetIndexArray().GetAt(global_vert_id + vert_idx);
								norm = pNormals->GetDirectArray().GetAt(id);
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
					
					v.n_x = (float)norm[0];
					v.n_y = (float)norm[1];
					v.n_z = (float)norm[2];
				}
				
				vertecies.push_back(v);
			}

		global_vert_id += polygon_size;
	}

	// create mesh on VRAM
	ICoreMesh *pCoreMesh = nullptr;
	
	MeshDataDesc vertDesc;
	vertDesc.pData = reinterpret_cast<uint8*>(&vertecies[0]);
	vertDesc.numberOfVertex = global_vert_id;
	vertDesc.positionOffset = 0;
	vertDesc.positionStride = sizeof(Vertex);
	vertDesc.normalsPresented = normal_element_count > 0;
	vertDesc.normalOffset = (normal_element_count > 0) * 12;
	vertDesc.normalStride = (normal_element_count > 0) * sizeof(Vertex);

	MeshIndexDesc indexDesc;
	indexDesc.pData = nullptr;
	indexDesc.number = 0;

	_pCoreRender->CreateMesh((ICoreMesh**)&pCoreMesh, &vertDesc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);

	if (pCoreMesh)
		meshes.push_back(pCoreMesh);
	else
		LOG_WARNING("[FBX]ResourceManager::_LogMesh(): Can not create mesh");


	/*
	for (int i = 0; i< normal_element_count; i++)
	{
		FbxString buff = "[FBX]";
		FbxGeometryElementNormal* ns = pMesh->GetElementNormal(i);

		LOG_FORMATTED("[FBX]Normals=%d", i);		

		switch (ns->GetMappingMode())
		{
			case FbxLayerElement::eByControlPoint:	buff += "EMappingMode=eByControlPoint "; break;
			case FbxLayerElement::eByPolygonVertex:	buff += "EMappingMode=eByPolygonVertex "; break;
			case FbxLayerElement::eNone:			buff += "EMappingMode=eNone "; break;
			case FbxLayerElement::eByPolygon:		buff += "EMappingMode=eByPolygon "; break;
			case FbxLayerElement::eByEdge:			buff += "EMappingMode=eByEdge "; break;
			case FbxLayerElement::eAllSame:			buff += "EMappingMode=eAllSame "; break;
			default:								buff += "EMappingMode=unknown "; break;;
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
	*/
}

void ResourceManager::_LogNodeTransform(FbxNode* pNode, int tabs)
{
	FbxVector4 lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);

	LOG_FORMATTED("[FBX] (eCamera) T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2]);
}
bool ResourceManager::_FBXLoad(IModel *&pModel, const char *pFileName, IProgressSubscriber *pPregress)
{
	FbxManager* lSdkManager = NULL;
	FbxScene* lScene = NULL;

	LOG("[FBX]Initializing FBX SDK...");

	_InitializeSdkObjects(lSdkManager, lScene);

	FbxString lFilePath(pFileName);

	LOG_FORMATTED("[FBX]Loading file: %s", lFilePath.Buffer());

	bool lResult = _LoadScene(lSdkManager, lScene, lFilePath.Buffer());

	if (!lResult)
		LOG_FATAL("[FBX]An error occurred while loading the scene...");
	else
	{
		_LogSceneHierarchy(pModel, lScene);
		//_ImportScene(lScene);
	}

	LOG("[FBX]Destroying FBX SDK...");
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
		case RENDER_MASTER::RES_TYPE::CORE_TEXTURE:
			return "CORE_TEXTURE";
		case RENDER_MASTER::RES_TYPE::CORE_SHADER:
			return "CORE_SHADER";
		case RENDER_MASTER::RES_TYPE::GAMEOBJECT:
			return "GAMEOBJECT";
		case RENDER_MASTER::RES_TYPE::MODEL:
			return "MODEL";
		case RENDER_MASTER::RES_TYPE::CAMERA:
			return "CAMERA";
	}

	return nullptr;
}

ResourceManager::ResourceManager()
{
	const char *pString = nullptr;

	_pCore->GetSubSystem((ISubSystem**)&_pFilesystem, SUBSYSTEM_TYPE::FILESYSTEM);
	
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Init()
{
	InitializeCriticalSection(&_cs);

	_pCore->GetSubSystem((ISubSystem**)&_pCoreRender, SUBSYSTEM_TYPE::CORE_RENDER);

	// create default plane mesh
	ICoreMesh *pPlane;

	float vertexPlane[12] = 
	{
		-1.0f, 1.0f, 0.0f,
		 1.0f,-1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f,
		 -1.0f, 1.0f, 0.0f
	};

	short indexPlane[6]
	{
		0, 1, 2,
		0, 2, 3
	};
	
	MeshDataDesc desc;
	desc.pData = reinterpret_cast<uint8*>(vertexPlane);
	desc.numberOfVertex = 4;

	MeshIndexDesc indexDesc;
	indexDesc.pData = reinterpret_cast<uint8*>(indexPlane);
	indexDesc.number = 6;
	indexDesc.format = MESH_INDEX_FORMAT::INT16;

	_pCoreRender->CreateMesh((ICoreMesh**)&pPlane, &desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);

	_default_resources.emplace_back(pPlane, 0, DEFAULT_RES_TYPE::PLANE);

	// create axes mesh
	ICoreMesh *pAxes;

	// position, color, position, color, ...
	float vertexAxes[] = {	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

	MeshDataDesc descAxes;
	descAxes.pData = reinterpret_cast<uint8*>(vertexAxes);
	descAxes.numberOfVertex = 6;
	descAxes.positionStride = 24;
	descAxes.colorPresented = true;
	descAxes.colorOffset = 12;
	descAxes.colorStride = 24;

	MeshIndexDesc indexEmpty;

	_pCoreRender->CreateMesh((ICoreMesh**)&pAxes, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES);

	_default_resources.emplace_back(pAxes, 0, DEFAULT_RES_TYPE::AXES);


	LOG("ResourceManager initalized");
}

API ResourceManager::GetName(OUT const char **pName)
{
	*pName = "ResourceManager";
	return S_OK;
}

API ResourceManager::LoadModel(OUT IModel **pModel, const char *pFileName, IProgressSubscriber *pProgress)
{
	const string file_ext = ToLowerCase(fs::path(pFileName).extension().string().erase(0, 1));
	
	char *pString;
	_pCore->GetDataDir(&pString);
	string dataPath = string(pString);

	string fullPath = dataPath + '\\' + string(pFileName);
	uint meshNumber;

	IModel *model{nullptr};

#ifdef USE_FBX
	if (file_ext == "fbx")
	{
		bool ret = _FBXLoad(model, fullPath.c_str(), pProgress);
		if (!ret)
			return S_FALSE;
	}
	else
#endif
	{
		LOG_FATAL_FORMATTED("ResourceManager::LoadModel unsupported format \"%s\"", file_ext.c_str());
		return S_FALSE;
	}

	AddToList(model);

	model->GetNumberOfMesh(&meshNumber);

	for (uint i = 0; i < meshNumber; i++)
	{
		ICoreMesh *pMesh;
		model->GetMesh(&pMesh, i);
		AddToList(pMesh);
	}

	ISceneManager *pSceneManager;
	_pCore->GetSubSystem((ISubSystem**)&pSceneManager, SUBSYSTEM_TYPE::SCENE_MANAGER);
	pSceneManager->AddGameObject(model);
	
	*pModel = model;

	return S_OK;
}

API ResourceManager::LoadShaderText(OUT ShaderText *pShader, const char *pVertName, const char *pGeomName, const char *pFragName)
{
	auto load_shader = [=](const char **&textOut, int& numLinesOut, const char *pName) -> APIRESULT
	{
		IFile *pFile = nullptr;
		uint fileSize = 0;
		string textIn;
		int filseExist = 0;
		
		char *pString;
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

		textIn.resize(fileSize);

		pFile->Read((uint8 *)textIn.c_str(), fileSize);
		pFile->CloseAndFree();

		split_by_eol(textOut, numLinesOut, textIn);
		
		return S_OK;
	};

	auto ret =	load_shader(pShader->pVertText, pShader->vertNumLines, pVertName);

	if (pGeomName)
		ret &=	load_shader(pShader->pGeomText, pShader->geomNumLines, pGeomName);

	ret &=		load_shader(pShader->pFragText, pShader->fragNumLines, pFragName);

	return ret;
}

API ResourceManager::GetDefaultResource(OUT IResource **pRes, DEFAULT_RES_TYPE type)
{
	if (type == DEFAULT_RES_TYPE::NONE)
	{
		pRes = nullptr;
		LOG_WARNING("ResourceManager::GetDefaultResource(): unknown type of resource");
		return S_FALSE;
	}

	auto it = std::find_if(_default_resources.begin(), _default_resources.end(), [type](const TDefaultResource& res) -> bool { return res.type == type; });

	assert(it != _default_resources.end());

	*pRes = it->pRes;
	it->refCount++;

	return S_OK;
}

API ResourceManager::AddToList(IResource *pResource)
{ 
	auto it = std::find_if(_resources.begin(), _resources.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });

	if (it == _resources.end())
	{
		_resources.push_back(TResource{ pResource, 1 });	
		DEBUG_LOG("AddToList(): added new resource! type=%s", LOG_TYPE::NORMAL, _resourceToStr(pResource));
	}
	else
	{
		it->refCount++;	
		DEBUG_LOG("AddToList(): refCount++ refCount==%i type=%s", LOG_TYPE::NORMAL, it->refCount, _resourceToStr(pResource));
	}

	return S_OK;
}

API ResourceManager::GetNumberOfResources(OUT uint *number)
{
	*number = (uint) _resources.size();
	return S_OK;
}

API ResourceManager::GetRefNumber(OUT uint *number, const IResource *pResource)
{
	auto it = std::find_if(_resources.begin(), _resources.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });

	if (it == _resources.end())
		return E_POINTER;

	assert(it->refCount > 0);

	*number = it->refCount;

	return S_OK;
}

API ResourceManager::DecrementRef(IResource *pResource)
{
	auto it = std::find_if(_resources.begin(), _resources.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
	
	if (it == _resources.end())
		return E_POINTER;

	assert(it->refCount > 0);

	it->refCount--;

	DEBUG_LOG("DecrementRef(): refCount-- refCount==%i type=%s", LOG_TYPE::NORMAL, it->refCount, _resourceToStr(pResource));

	return S_OK;
}

API ResourceManager::RemoveFromList(IResource *pResource)
{
	uint refCount;
	GetRefNumber(&refCount, pResource);

	if (refCount == 1)
	{
		auto it = std::remove_if(_resources.begin(), _resources.end(), [pResource](const TResource& res) -> bool { return res.pRes == pResource; });
		_resources.erase(it, _resources.end());

		DEBUG_LOG("RemoveFromList(): deleted! type=%s", LOG_TYPE::NORMAL, _resourceToStr(pResource));
	}
	else
		LOG_WARNING_FORMATTED("RemoveFromList(): not deleted! refNumber=%i type=%s", refCount, _resourceToStr(pResource));

	return S_OK;
}

API ResourceManager::FreeAllResources()
{
	DEBUG_LOG("ResourceManager::FreeAllResources(): resources total=%i", LOG_TYPE::NORMAL, _resources.size());

	// first free all resources that have refCount = 1
	// and so on...
	while (!_resources.empty())
	{
		vector<TResource> one_ref_res;

		auto ref_is_1 = [](const TResource& res) {return res.refCount == 1; };

		// partition: 
		// all elements that should not be moved come before 
		// all elements that should be moved come after
		// stable_partition maintains relative order in each group
		auto p = std::stable_partition(_resources.begin(), _resources.end(), [&](const auto& x) { return !ref_is_1(x); });

		// move range elements that sould be removed
		one_ref_res.insert(one_ref_res.end(), std::make_move_iterator(p), std::make_move_iterator(_resources.end()));
		
		// erase the moved-from elements.
		// no need this because pRes->Free() removes himself from the vector!
		//_resources.erase(p, _resources.end());
		
		#ifdef _DEBUG
			static int i = 0;
			i++;
			if (i > 20) return S_FALSE; // occured some error. maybe circular references => in debug limit number of iterations
			auto res_before = _resources.size();
		#endif

		// free resources
		for (auto& res : one_ref_res)
			res.pRes->Free();

		#ifdef _DEBUG
			auto res_deleted = res_before - _resources.size();
			int res_deleted_percent = (int)(100 * ((float)res_deleted / res_before));
			DEBUG_LOG("FreeAllResources(): (iteration=%i) : to delete=%i  deleted=%i (%i%%) resources left=%i", LOG_TYPE::NORMAL, i, one_ref_res.size(), res_deleted, res_deleted_percent, _resources.size());
		#endif
	}

	// free deafult resources
	for (auto& res : _default_resources)
		res.pRes->Free();
	
	return S_OK;
}
