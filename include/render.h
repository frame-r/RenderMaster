#pragma once
#include "Common.h"

struct ViewData
{
	mat4 cameraProjUnjitteredMat_;
	mat4 cameraProjMat_;
	mat4 cameraViewMat_;
	mat4 cameraViewProjMat_;
	vec4 cameraWorldPos_;
	mat4 cameraViewProjectionInvMat_;
	mat4 cameraViewInvMat_;
};

class Render
{
	struct RenderBuffers
	{
		Texture* color;
		Texture* color_reprojected;
		Texture* albedo;
		Texture* normal;
		Texture* shading;
		Texture* depth;
		Texture* diffuseLight;
		Texture* specularLight;
		Texture* velocity;
	};

	struct RenderMesh
	{
		int modelId;
		Mesh* mesh{};
		Material* mat{};
		mat4 worldTransformMat;
		mat4 worldTransformMatPrev;
	};

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
	mat4 cameraProjUnjitteredMat_;
	mat4 cameraProjMat_;
	mat4 cameraViewMat_;
	mat4 cameraViewProjMat_;
	vec4 cameraWorldPos_;
	mat4 cameraViewProjectionInvMat_;
	mat4 cameraViewInvMat_;

	// prev
	mat4 cameraPrevProjUnjitteredMat_;
	mat4 cameraPrevProjMat_;
	mat4 cameraPrevViewMat_;
	mat4 cameraPrevViewProjMat_;
	vec4 cameraPrevWorldPos_;
	mat4 cameraPrevViewProjectionInvMat_;
	mat4 cameraPrevViewInvMat_;

	mat4 cameraPrevViewProjMatRejittered_; // previous Projection matrix with same jitter as current frame


	// Render internal resources
	ManagedPtr<Texture> fontTexture;
	ManagedPtr<Texture> environmentTexture;
	Texture* whiteTexture;
	ManagedPtr<Mesh> planeMesh;
	ManagedPtr<Mesh> gridMesh;
	ManagedPtr<Mesh> lineMesh;

	Material* compositeMaterial{};
	Material* copyMaterial{};

	struct RenderVector
	{
		vec4 color;
		vec3 v;
	};
	std::vector<RenderVector> renderVectors;

	float diffuseEnvironemnt{1.0f};
	float specularEnvironemnt{1.0f};
	int specualrQuality{};
	VIEW_MODE viewMode{VIEW_MODE::FINAL};
	float debugParam{1};
	float debugParam1{1};

	bool taa{true};

	std::vector<RenderMesh> getRenderMeshes();
	RenderScene getRenderScene();
	void renderGrid();
	void drawMeshes(PASS pass, std::vector<RenderMesh>& meshes);

public:
	void Init();
	void Update();
	void Free();
	void RenderFrame(size_t viewID, const mat4& ViewMat, const mat4& ProjMat);
	auto GetPrevRenderTexture(PREV_TEXTURES id, uint width, uint height, TEXTURE_FORMAT format) -> Texture*;
	void ExchangePrevRenderTexture(Texture *prev, Texture *some);

public:
	auto DLLEXPORT GetShader(const char* path, Mesh* meshattrib = nullptr, const std::vector<std::string>& defines = std::vector<std::string>()) -> Shader*;
	auto DLLEXPORT ReloadShaders() -> void;
	auto DLLEXPORT RenderGUI() -> void;
	auto DLLEXPORT DrawMeshes(PASS pass) -> void;
	auto DLLEXPORT GetRenderTexture(uint width, uint height, TEXTURE_FORMAT format) -> Texture*;
	auto DLLEXPORT ReleaseRenderTexture(Texture *tex) -> void;
	auto DLLEXPORT RenderVector(const vec3& end, const vec4& c) -> void { renderVectors.push_back({c, end}); }

	auto DLLEXPORT SetDiffuseEnvironemnt(float v) -> void { diffuseEnvironemnt = v; }
	auto DLLEXPORT GetDiffuseEnvironemnt() -> float { return diffuseEnvironemnt; }
	auto DLLEXPORT SetSpecularEnvironemnt(float v) -> void { specularEnvironemnt = v; }
	auto DLLEXPORT GetSpecularEnvironemnt() -> float { return specularEnvironemnt; }
	auto DLLEXPORT SetSpecularQuality(int value) -> void { specualrQuality = value; }
	auto DLLEXPORT GetSpecularQuality() -> int { return specualrQuality; }
	auto DLLEXPORT SetViewMode(VIEW_MODE value) -> void { viewMode = value; }
	auto DLLEXPORT GetViewMode() -> VIEW_MODE { return viewMode; }
	auto DLLEXPORT IsTAA() -> bool { return taa; }
	auto DLLEXPORT SetTAA(bool value) -> void { taa = value; }

	// debug
	auto DLLEXPORT SetDebugParam(float v) -> void { debugParam = v; }
	auto DLLEXPORT SetDebugParam1(float v) -> void { debugParam1 = v; }
};

