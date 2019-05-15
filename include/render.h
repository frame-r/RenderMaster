#pragma once
#include "Common.h"

class Camera;
class Mesh;
class Model;
class StructuredBuffer;
struct ShaderRequirement;
class Shader;
class Texture;
class Light;

struct RenderBuffers
{
	Texture *albedo;
	Texture *normal;
	Texture *shading;
	Texture *depth;
	Texture *diffuseLight;
	Texture *specularLight;
};

struct RenderMesh
{
	int modelId;
	Mesh *mesh{nullptr};
	mat4 worldTransformMat;
	vec4 color;
	vec4 shading; // r - roughness, g - metallic
	Texture *albedoTex;
};


class Render
{
	struct RenderLight
	{
		Light *light;
		vec3 worldDirection;
	};

	struct RenderScene
	{
		std::vector<RenderMesh> meshes;
		std::vector<RenderLight> lights;
		bool hasWorldLight;
	};


	// Frame data
	mat4 ViewMat_;
	mat4 ViewProjMat_;
	vec4 CameraWorldPos_;
	mat4 CameraViewProjectionInv_;
	mat4 CameraViewInv_;

	ManagedPtr<Texture> fontTexture;
	ManagedPtr<Texture> environmentTexture;

	Texture* whiteTexture;

	ManagedPtr<Mesh> planeMesh;
	ManagedPtr<Mesh> gridMesh;
	ManagedPtr<Mesh> lineMesh;

	struct RenderVector
	{
		vec4 color;
		vec3 v;
	};
	std::vector<RenderVector> vectors;

	float diffuseEnvironemnt{1.0f};
	float specularEnvironemnt{1.0f};

	float debugParam{1};
	float debugParam1{1};

	std::vector<RenderMesh> getRenderMeshes();
	RenderScene getRenderScene();
	void renderGrid();
	void drawMeshes(const char *shader, PASS pass, std::vector<RenderMesh>& meshes);

public:
	void Init();
	void Update();
	void Free();
	void RenderFrame(const mat4& ViewMat, const mat4& ProjMat);

public:
	auto DLLEXPORT GetShader(const ShaderRequirement &req) -> Shader*;
	auto DLLEXPORT ReloadShaders() -> void;
	auto DLLEXPORT RenderGUI() -> void;
	auto DLLEXPORT DrawMeshes(const char *shader, PASS pass) -> void;
	auto DLLEXPORT GetRenderTexture(uint width, uint height, TEXTURE_FORMAT format) -> Texture*;
	auto DLLEXPORT ReleaseRenderTexture(Texture *tex) -> void;
	auto DLLEXPORT RenderVector(const vec3& end, const vec4& c) -> void { vectors.push_back({c, end}); }

	auto DLLEXPORT SetDiffuseEnvironemnt(float v) -> void { diffuseEnvironemnt = v; }
	auto DLLEXPORT GetDiffuseEnvironemnt() -> float { return diffuseEnvironemnt; }
	auto DLLEXPORT SetSpecularEnvironemnt(float v) -> void { specularEnvironemnt = v; }
	auto DLLEXPORT GetSpecularEnvironemnt() -> float { return specularEnvironemnt; }

	// debug
	auto DLLEXPORT SetDebugParam(float v) -> void { debugParam = v; }
	auto DLLEXPORT SetDebugParam1(float v) -> void { debugParam1 = v; }
};

