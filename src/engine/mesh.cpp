#include "pch.h"
#include "mesh.h"
#include "core.h"
#include "console.h"
#include "filesystem.h"
#include "icorerender.h"

Mesh::Mesh(const std::string& path) : path_(path)
{
}
Mesh::~Mesh()
{
	Log("Mesh destroyed: '%s'", path_.c_str());
}
ICoreMesh* createStdMesh(const char *path)
{
	ICoreMesh *ret = nullptr;

	if (!strcmp(path, "std#plane"))
	{
		float vertex[40] =
		{
			-1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		0.0f, 1.0f,
			 1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		1.0f, 0.0f,
			 1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		1.0f, 1.0f,
			-1.0f,-1.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f, 0.0f,		0.0f, 0.0f
		};
	
		unsigned short indexPlane[6]
		{
			0, 2, 1,
			0, 1, 3
		};

		MeshDataDesc desc;
		desc.pData = reinterpret_cast<uint8*>(vertex);
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

		ret = CORE_RENDER->CreateMesh(&descArrows, &indexEmpty, VERTEX_TOPOLOGY::TRIANGLES);
	}

	return ret;
}


bool Mesh::Load()
{
	Log("Mesh loading: '%s'", path_.c_str());

	ICoreMesh *m = createStdMesh(path_.c_str());

	if (m)
	{
		coreMesh_.reset(m);
		return true;
	}

	MeshHeader header;

	if (!FS->FileExist(path_.c_str()))
	{
		LogCritical("Mesh::Load(): file '%s' not found", path_);
		return false;
	}

	File f = FS->OpenFile(path_.c_str(), FILE_OPEN_MODE::READ | FILE_OPEN_MODE::BINARY);
	f.Read(reinterpret_cast<uint8*>(&header), sizeof(MeshHeader));

	int is_positions = (header.attributes & 1u) > 0;
	int is_normals = (header.attributes & 2u) > 0;
	int is_uv = (header.attributes & 4u) > 0;
	int is_tangent = (header.attributes & 8u) > 0;
	int is_binormal = (header.attributes & 16u) > 0;
	int is_color = (header.attributes & 32u) > 0;

	size_t bytes = 0;
	bytes += is_positions * header.numberOfVertex * sizeof(vec4);
	bytes += is_normals * header.numberOfVertex * sizeof(vec4);
	bytes += is_uv * header.numberOfVertex * sizeof(vec2);
	bytes += is_tangent * header.numberOfVertex * sizeof(vec4);
	bytes += is_binormal * header.numberOfVertex * sizeof(vec4);
	bytes += is_color * header.numberOfVertex * sizeof(vec4);

	unique_ptr<float[]> data = unique_ptr<float[]>(new float[bytes / 4]);
	f.Read(reinterpret_cast<uint8*>(data.get()), bytes);

	MeshDataDesc desc;
	desc.numberOfVertex = header.numberOfVertex;
	desc.positionOffset = header.positionOffset;
	desc.positionStride = header.positionStride;
	desc.normalsPresented = is_normals;
	desc.normalOffset = header.normalOffset;
	desc.normalStride = header.normalOffset;
	desc.tangentPresented = is_tangent;
	desc.tangentOffset = header.tangentOffset;
	desc.tangentStride = header.tangentStride;
	desc.binormalPresented = is_binormal;
	desc.binormalOffset = header.binormalOffset;
	desc.binormalStride = header.binormalStride;
	desc.texCoordPresented = is_uv;
	desc.texCoordOffset = header.uvOffset;
	desc.texCoordStride = header.uvStride;
	desc.colorPresented = is_color;
	desc.colorOffset = header.colorOffset;
	desc.colorStride = header.colorStride;
	desc.pData = reinterpret_cast<uint8*>(data.get());

	MeshIndexDesc indexDesc;

	m = CORE_RENDER->CreateMesh(&desc, &indexDesc, VERTEX_TOPOLOGY::TRIANGLES);

	if (!m)
	{
		LogCritical("Mesh::Load(): error occured");
		return false;
	}

	coreMesh_.reset(m);

	return true;
}

auto DLLEXPORT Mesh::GetAttributes() -> INPUT_ATTRUBUTE
{
	return coreMesh_->GetAttributes();
}

auto DLLEXPORT Mesh::GetVideoMemoryUsage() -> size_t
{
	if (!coreMesh_)
		return 0;

	return coreMesh_->GetVideoMemoryUsage();
}
