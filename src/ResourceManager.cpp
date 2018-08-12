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
	FbxString lString;

	LOG("[FBX]Scene hierarchy:");

	FbxNode* lRootNode = pScene->GetRootNode();
	_LogNode(meshes, lRootNode, 0);

	//_LogNodeTransform(lRootNode, ("[FBX] (root node!) " + lString + lRootNode->GetName()).Buffer());
	//for (int i = 0; i < lRootNode->GetChildCount(); i++)
	//	_LogNode(meshes, lRootNode->GetChild(i), 0);

	if (meshes.size() == 0)
		LOG_WARNING("[FBX]No meshes loaded");

	pModel = new Model(meshes);
}

void ResourceManager::_LogNode(vector<ICoreMesh *>& meshes, FbxNode* pNode, int depth)
{
	FbxString lString;
	FbxNodeAttribute* node = pNode->GetNodeAttribute();
	FbxNodeAttribute::EType lAttributeType = FbxNodeAttribute::eUnknown;
	if (node) lAttributeType = node->GetAttributeType();

	switch (lAttributeType)
	{
		case FbxNodeAttribute::eMesh:		_LogMesh(meshes, (FbxMesh*)pNode->GetNodeAttribute(), pNode); break;
		case FbxNodeAttribute::eMarker:		LOG(("[FBX] (eMarker) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eSkeleton:	LOG(("[FBX] (eSkeleton) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eNurbs:		LOG(("[FBX] (eNurbs) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::ePatch:		LOG(("[FBX] (ePatch) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eCamera:		_LogNodeTransform(pNode, ("[FBX] (eCamera) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLight:		LOG(("[FBX] (eLight) " + lString + pNode->GetName()).Buffer()); break;
		case FbxNodeAttribute::eLODGroup:	LOG(("[FBX] (eLODGroup) " + lString + pNode->GetName()).Buffer()); break;
		default:							_LogNodeTransform(pNode, ("[FBX] (unknown!) " + lString + pNode->GetName()).Buffer()); break;
	}

	int childs = pNode->GetChildCount();
	if (childs)
	{
		LOG_FORMATTED("[FBX] for node=%s childs=%i", pNode->GetName(), childs);
		for (int i = 0; i < childs; i++)
			_LogNode(meshes, pNode->GetChild(i), depth + 1);
	}
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

	//FbxDouble3 tr = pNode->LclTranslation.Get();
	//FbxDouble3 rot = pNode->LclRotation.Get();
	//FbxDouble3 sc = pNode->LclScaling.Get();
	FbxVector4 tr = pNode->EvaluateGlobalTransform().GetT();
	FbxVector4 rot = pNode->EvaluateGlobalTransform().GetR();
	FbxVector4 sc = pNode->EvaluateGlobalTransform().GetS();
	//LOG_FORMATTED("[FBX]T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2], lTmpVector[0], lTmpVector[1], lTmpVector[2]);

	DEBUG_LOG_FORMATTED("[FBX] (eMesh) %-10.10s T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f) CP=%5d POLYS=%5d NORMAL=%d UV=%d TANG=%d BINORM=%d", 
		pNode->GetName(),
		tr[0], tr[1], tr[2], rot[0], rot[1], rot[2], sc[0], sc[1], sc[2],
		control_points_count, polygon_count, normal_element_count, uv_layer_count, tangent_layers_count, binormal_layers_count);

	struct Vertex
	{
		float x, y, z;
		float n_x, n_y, n_z;
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
}

void ResourceManager::_LogNodeTransform(FbxNode* pNode, const char *str)
{
	FbxVector4 tr = pNode->EvaluateGlobalTransform().GetT();
	FbxVector4 rot = pNode->EvaluateGlobalTransform().GetR();
	FbxVector4 sc = pNode->EvaluateGlobalTransform().GetS();

	DEBUG_LOG_FORMATTED("%s T=(%.1f %.1f %.1f) R=(%.1f %.1f %.1f) S=(%.1f %.1f %.1f)", str, tr[0], tr[1], tr[2], rot[0], rot[1], rot[2], sc[0], sc[1], sc[2]);
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
	MeshDataDesc descAxes;
	MeshIndexDesc indexEmpty;
	ICoreMesh *pAxes;
	ICoreMesh *pPlane;

	InitializeCriticalSection(&_cs);

	_pCore->GetSubSystem((ISubSystem**)&_pCoreRender, SUBSYSTEM_TYPE::CORE_RENDER);

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

	if (SUCCEEDED(_pCoreRender->CreateMesh((ICoreMesh**)&pPlane, &desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES)))
		_resources[pPlane] = TResource{pPlane, 0, DEFAULT_RES_TYPE::PLANE};

	//
	// Create axes
	//
	
	// position, color, position, color, ...
	float vertexAxes[] = {	0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };
	
	descAxes.pData = reinterpret_cast<uint8*>(vertexAxes);
	descAxes.numberOfVertex = 6;
	descAxes.positionStride = 24;
	descAxes.colorPresented = true;
	descAxes.colorOffset = 12;
	descAxes.colorStride = 24;	

	if (SUCCEEDED(_pCoreRender->CreateMesh((ICoreMesh**)&pAxes, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES)))
		_resources[pAxes] = TResource{pAxes, 0, DEFAULT_RES_TYPE::AXES};
	
	
	//
	// Create grid
	//
	constexpr float linesInterval = 5.0f;
	constexpr int linesNumber = 31;
	constexpr float startOffset = linesInterval * (linesNumber / 2);
	//position, color, position, color, ...
	vec3 vertexGrid[4 * linesNumber];

	for (int i = 0; i < linesNumber; i++)
	{
		vertexGrid[i * 4] = vec3(-startOffset + i * linesInterval, -startOffset, 0.0f);
		vertexGrid[i * 4 + 1] = vec3(-startOffset + i * linesInterval, startOffset, 0.0f);
		vertexGrid[i * 4 + 2] = vec3(startOffset , -startOffset + i * linesInterval, 0.0f);
		vertexGrid[i * 4 + 3] = vec3(-startOffset , -startOffset + i * linesInterval, 0.0f);
	}
	
	descAxes.pData = reinterpret_cast<uint8*>(vertexGrid);
	descAxes.numberOfVertex = 4 * linesNumber;
	descAxes.positionStride = 12;
	descAxes.colorPresented = false;
	descAxes.colorOffset = 0;
	descAxes.colorStride = 0;

	if (SUCCEEDED(_pCoreRender->CreateMesh((ICoreMesh**)&pAxes, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES)))
		_resources[pAxes] = TResource{pAxes, 0, DEFAULT_RES_TYPE::GRID};


	//
	// Create axes arrows
	//
	// position, color, position, color, ...
	constexpr int segments = 12;
	constexpr float arrowRadius = 0.065f;
	constexpr float arrowLength = 0.3f;
	constexpr int numberOfVeretex = 3 * 3 * segments;
	constexpr int floats = (3 + 3) * numberOfVeretex;
	float vertexAxesArrows[floats];

	for (int i = 0; i < 3; i++) // 3 axes
	{
		vec3 color;
		color.xyz[i] = 1.0f;
		for (int j = 0; j < segments; j++) 
		{
			constexpr float pi2 = 3.141592654f * 2.0f;
			float alpha = pi2 * (float(j) / segments);
			float dAlpha = pi2 * (1.0f / segments);

			vec3 v1, v2, v3;

			v1.xyz[i] = 1.0f + arrowLength;

			v2.xyz[i] = 1.0f;
			v2.xyz[(i + 1) % 3] = cos(alpha) * arrowRadius;
			v2.xyz[(i + 2) % 3] = sin(alpha) * arrowRadius;

			v3.xyz[i] = 1.0f;
			v3.xyz[(i + 1) % 3] = cos(alpha + dAlpha) * arrowRadius;
			v3.xyz[(i + 2) % 3] = sin(alpha + dAlpha) * arrowRadius;

			memcpy(vertexAxesArrows + i * segments * 18 + j * 18, &v1.x, 12);
			memcpy(vertexAxesArrows + i * segments * 18 + j * 18 + 3, &color.x, 12);
			memcpy(vertexAxesArrows + i * segments * 18 + j * 18 + 6, &v2.x, 12);
			memcpy(vertexAxesArrows + i * segments * 18 + j * 18 + 9, &color.x, 12);
			memcpy(vertexAxesArrows + i * segments * 18 + j * 18 + 12, &v3.x, 12);
			memcpy(vertexAxesArrows + i * segments * 18 + j * 18 + 15, &color.x, 12);
		}
	}

	descAxes.pData = reinterpret_cast<uint8*>(vertexAxesArrows);
	descAxes.numberOfVertex = numberOfVeretex;
	descAxes.positionStride = 24;
	descAxes.colorPresented = true;
	descAxes.colorOffset = 12;
	descAxes.colorStride = 24;

	if (SUCCEEDED(_pCoreRender->CreateMesh((ICoreMesh**)&pAxes, &descAxes, &indexEmpty, VERTEX_TOPOLOGY::TRIANGLES)))
		_resources[pAxes] = TResource{pAxes, 0, DEFAULT_RES_TYPE::AXES_ARROWS};


	LOG("Resource Manager initalized");
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
	pSceneManager->AddRootGameObject(model);
	
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

API ResourceManager::GetDefaultResource(OUT IResource **pResource, DEFAULT_RES_TYPE type)
{
	if (type == DEFAULT_RES_TYPE::CUSTOM)
	{
		pResource = nullptr;
		LOG_WARNING("ResourceManager::GetDefaultResource(): unknown type of resource");
		return S_FALSE;
	}

	auto it = std::find_if(_resources.begin(), _resources.end(), [type](const std::pair<const IResource *, TResource>& res) -> bool { return res.second.type == type; });

	assert(it != _resources.end());

	*pResource = it->second.pRes;
	it->second.refCount++;

	return S_OK;
}

API ResourceManager::AddToList(IResource *pResource)
{ 
	assert(pResource != nullptr && "ResourceManager::AddToList(): pResource==nullptr");
	auto it = _resources.find(pResource);

	if (it == _resources.end())
	{
		_resources[pResource] = TResource{pResource, 1, DEFAULT_RES_TYPE::CUSTOM};
		DEBUG_LOG("ResourceManager::AddToList(): added new resource! type=%s", LOG_TYPE::NORMAL, _resourceToStr(pResource));
	}
	else
	{
		it->second.refCount++;
		DEBUG_LOG("ResourceManager::AddToList(): refCount++ refCount==%i type=%s", LOG_TYPE::NORMAL, it->second.refCount, _resourceToStr(pResource));
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
	auto it = _resources.find(pResource);

	if (it == _resources.end())
	{
		*number = 0;
		return E_POINTER;
	}
	
	*number = it->second.refCount;

	return S_OK;
}

API ResourceManager::DecrementRef(IResource *pResource)
{
	auto it = _resources.find(pResource);
	
	if (it == _resources.end())
		return E_POINTER;

	assert(it->second.refCount > 0);

	it->second.refCount--;

	DEBUG_LOG("ResourceManager::DecrementRef(): refCount-- refCount==%i type=%s", LOG_TYPE::NORMAL, it->second.refCount, _resourceToStr(pResource));

	return S_OK;
}

API ResourceManager::RemoveFromList(IResource *pResource)
{
	uint refCount;
	GetRefNumber(&refCount, pResource);

	if (refCount == 1)
	{
		_resources.erase(pResource);
		DEBUG_LOG("ResourceManager::RemoveFromList(): deleted! type=%s", LOG_TYPE::NORMAL, _resourceToStr(pResource));
	}
	else
	{
		_resources.erase(pResource);
		LOG_WARNING_FORMATTED("ResourceManager::RemoveFromList(): deleted! type=%s. may be warning(refNumber = %i)!", _resourceToStr(pResource), refCount);
	}

	return S_OK;
}

API ResourceManager::FreeAllResources()
{
	DEBUG_LOG("ResourceManager::FreeAllResources(): resources total=%i", LOG_TYPE::NORMAL, _resources.size());

	// first free all resources that have refCount = 1
	// and so on...
	while (!_resources.empty())
	{
		vector<IResource*> one_ref_res;
		
		for (auto& res : _resources)
		{
			if (res.second.refCount <= 1)
				one_ref_res.push_back(res.second.pRes);
		}
		
		#ifdef _DEBUG
			static int i = 0;
			i++;
			if (i > 20) return S_FALSE; // occured some error. maybe circular references => in debug limit number of iterations
			auto res_before = _resources.size();
		#endif

		// free resources
		for (auto* pRes : one_ref_res)
		{
			auto it = _resources.find(pRes);
			if (it != _resources.end())
			{
				pRes->Free();
				_resources.erase(pRes);
			}
		}

		#ifdef _DEBUG
			auto res_deleted = res_before - _resources.size();
			int res_deleted_percent = (int)(100 * ((float)res_deleted / res_before));
			DEBUG_LOG("ResourceManager::FreeAllResources(): (iteration=%i) : to delete=%i  deleted=%i (%i%%) resources left=%i", LOG_TYPE::NORMAL, i, one_ref_res.size(), res_deleted, res_deleted_percent, _resources.size());
		#endif
	}
		
	return S_OK;
}
