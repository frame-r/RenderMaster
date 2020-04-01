#include "pch.h"
#include "mesh.h"
#include "core.h"
#include "console.h"
#include "filesystem.h"
#include "icorerender.h"

static float vertexPlane[40] =
{
	-1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
	 1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
	 1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
	-1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		0.0f, 0.0f
};

static unsigned short indexPlane[6]
{
	0, 2, 1,
	0, 1, 3
};


static const char* stdMeshses[] =
{
	"std#plane",
	"std#grid",
	"std#line",
	"std#axes_arrows"
};

bool Mesh::isSphere()
{
	return strcmp(path_.c_str(), "standard\meshes\sphere.mesh") == 0;
}

bool Mesh::isPlane()
{
	return strcmp(path_.c_str(), "std#plane") == 0;
}

bool Mesh::isStd()
{
	for (int i = 0; i < _countof(stdMeshses); ++i)
		if (strcmp(path_.c_str(), stdMeshses[i]) == 0)
			return true;
	return false;
}

Mesh::Mesh(const std::string& path) : path_(path)
{
}

Mesh::~Mesh()
{
	Log("Mesh unloaded: '%s'", path_.c_str());
}

ICoreMesh* createStdMesh(const char *path)
{
	ICoreMesh *ret = nullptr;

	if (!strcmp(path, "std#plane"))
	{
		MeshDataDesc desc;
		desc.pData = reinterpret_cast<uint8*>(vertexPlane);
		desc.numberOfVertex = 4;
		desc.positionStride = 40;
		desc.normalsPresented = true;
		desc.normalOffset = 16;
		desc.normalStride = 40;
		desc.texCoordPresented = true;
		desc.texCoordOffset = 32;
		desc.texCoordStride = 40;

		MeshIndexDesc indexDesc;
		indexDesc.pData = reinterpret_cast<uint8*>(indexPlane);
		indexDesc.number = 6;
		indexDesc.format = MESH_INDEX_FORMAT::INT16;

		ret = CORE_RENDER->CreateMesh(&desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);
	}
	else if (!strcmp(path, "std#grid"))
	{
		const float linesInterval = 10.0f;
		const int linesNumber = 21;
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

		ret = CORE_RENDER->CreateMesh(&descGrid, &indexEmpty, VERTEX_TOPOLOGY::LINES);
	}
	else if (!strcmp(path, "std#line"))
	{
		float vertexAxes[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
		MeshIndexDesc indexEmpty;
		MeshDataDesc descAxes;
		descAxes.pData = reinterpret_cast<uint8*>(vertexAxes);
		descAxes.numberOfVertex = 2;
		descAxes.positionStride = 16;

		ret = CORE_RENDER->CreateMesh(&descAxes, &indexEmpty, VERTEX_TOPOLOGY::LINES);
	}
	else if (!strcmp(path, "std#axes_arrows"))
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

				memcpy(vertexAxesArrows + j * 12 + 0L, &v1.x, 16);
				memcpy(vertexAxesArrows + j * 12 + 4L, &v2.x, 16);
				memcpy(vertexAxesArrows + j * 12 + 8L, &v3.x, 16);
			}
		}
		MeshIndexDesc indexEmpty;
		MeshDataDesc descArrows;
		descArrows.pData = reinterpret_cast<uint8*>(vertexAxesArrows);
		descArrows.numberOfVertex = numberOfVeretex;
		descArrows.positionStride = 16;

		ret = CORE_RENDER->CreateMesh(&descArrows, &indexEmpty, VERTEX_TOPOLOGY::TRIANGLES);
	}

	return ret;
}

bool Mesh::Load()
{
	if (isStd())
	{
		coreMeshPtr.reset(createStdMesh(path_.c_str()));
		return true;
	}

	Log("Mesh loading: '%s'", path_.c_str());

	if (!FS->FileExist(path_.c_str()))
	{
		LogCritical("Mesh::Load(): file '%s' not found", path_);
		return false;
	}

	FileMapping mappedFile = FS->CreateMemoryMapedFile(path_.c_str());

	MeshHeader header;
	memcpy(&header, mappedFile.ptr, sizeof(header));

	int bPositions = (header.attributes & 1u) > 0;
	int bNormals = (header.attributes & 2u) > 0;
	int bUv = (header.attributes & 4u) > 0;
	int bTangent = (header.attributes & 8u) > 0;
	int bBinormal = (header.attributes & 16u) > 0;
	int bColor = (header.attributes & 32u) > 0;

	size_t bytes = 0;
	bytes += (size_t)bPositions * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bNormals * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bUv * header.numberOfVertex * sizeof(vec2);
	bytes += (size_t)bTangent * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bBinormal * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bColor * header.numberOfVertex * sizeof(vec4);

	MeshDataDesc desc;
	desc.numberOfVertex = header.numberOfVertex;
	desc.positionOffset = header.positionOffset;
	desc.positionStride = header.positionStride;
	desc.normalsPresented = bNormals;
	desc.normalOffset = header.normalOffset;
	desc.normalStride = header.normalOffset;
	desc.tangentPresented = bTangent;
	desc.tangentOffset = header.tangentOffset;
	desc.tangentStride = header.tangentStride;
	desc.binormalPresented = bBinormal;
	desc.binormalOffset = header.binormalOffset;
	desc.binormalStride = header.binormalStride;
	desc.texCoordPresented = bUv;
	desc.texCoordOffset = header.uvOffset;
	desc.texCoordStride = header.uvStride;
	desc.colorPresented = bColor;
	desc.colorOffset = header.colorOffset;
	desc.colorStride = header.colorStride;
	desc.pData = reinterpret_cast<uint8*>(mappedFile.ptr + sizeof(MeshHeader));

	MeshIndexDesc indexDesc;

	if (auto coreMesh = CORE_RENDER->CreateMesh(&desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES))
		coreMeshPtr.reset(coreMesh);
	else
	{
		LogCritical("Mesh::Load(): error occured");
		return false;
	}	

	center_.x = header.minX * 0.5f + header.maxX * 0.5f;
	center_.y = header.minY * 0.5f + header.maxY * 0.5f;
	center_.z = header.minZ * 0.5f + header.maxZ * 0.5f;

	return true;
}

std::shared_ptr<RaytracingData> Mesh::GetRaytracingData()
{
	if (trianglesDataObjectSpace)
		return trianglesDataObjectSpace;

	if (isStd())
	{
		if (isPlane())
		{
			trianglesDataObjectSpace = shared_ptr<RaytracingData>(new RaytracingData);
			trianglesDataObjectSpace->triangles.resize(6);

			vector<vec4>& trinagles = trianglesDataObjectSpace->triangles;

			for (int i = 0; i < _countof(indexPlane); ++i)
			{
				trinagles[i] = vec4(vertexPlane[i * 10], vertexPlane[i * 10 + 1], vertexPlane[i * 10 + 2], vertexPlane[i * 10 + 3]);
			}

			return trianglesDataObjectSpace;

		} else
			throw new std::exception("not impl");
	}
	if (!FS->FileExist(path_.c_str()))
	{
		LogCritical("Mesh::Load(): file '%s' not found", path_);
		trianglesDataObjectSpace = nullptr;
		return trianglesDataObjectSpace;
	}

	FileMapping mappedFile = FS->CreateMemoryMapedFile(path_.c_str());

	MeshHeader header;
	memcpy(&header, mappedFile.ptr, sizeof(header));

	int bPositions = (header.attributes & 1u) > 0;
	int bNormals = (header.attributes & 2u) > 0;
	int bUv = (header.attributes & 4u) > 0;
	int bTangent = (header.attributes & 8u) > 0;
	int bBinormal = (header.attributes & 16u) > 0;
	int bColor = (header.attributes & 32u) > 0;

	size_t bytes = 0;
	bytes += (size_t)bPositions * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bNormals * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bUv * header.numberOfVertex * sizeof(vec2);
	bytes += (size_t)bTangent * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bBinormal * header.numberOfVertex * sizeof(vec4);
	bytes += (size_t)bColor * header.numberOfVertex * sizeof(vec4);

	MeshDataDesc desc;
	desc.numberOfVertex = header.numberOfVertex;
	desc.positionOffset = header.positionOffset;
	uint8_t *data = reinterpret_cast<uint8_t*>(mappedFile.ptr + sizeof(MeshHeader) + desc.positionOffset);

	assert(desc.numberOfVertex % 3 == 0);

	trianglesDataObjectSpace = shared_ptr<RaytracingData>(new RaytracingData);
	trianglesDataObjectSpace->triangles.resize(desc.numberOfVertex);
	
	vector<vec4>& in = trianglesDataObjectSpace->triangles;

	uint32_t stride = header.positionStride;
	triangles = desc.numberOfVertex / 3;

	for (int i = 0; i < desc.numberOfVertex; ++i)
	{
		in[i] = (vec4)*data;
		data += stride;
	}

	return trianglesDataObjectSpace;
}

auto DLLEXPORT Mesh::GetAttributes() -> INPUT_ATTRUBUTE
{
	return coreMeshPtr->GetAttributes();
}

auto DLLEXPORT Mesh::GetVideoMemoryUsage() -> size_t
{
	if (!coreMeshPtr)
		return 0;

	return coreMeshPtr->GetVideoMemoryUsage();
}

auto DLLEXPORT Mesh::GetCenter() -> vec3
{
	return center_;
}
