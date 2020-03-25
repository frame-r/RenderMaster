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

class Render : public IProfilerCallback
{

	enum TIMER_ID
	{
		T_GBUFFER = 0,
		T_LIGHTS,
		T_COMPOSITE,
		T_ALL_FRAME
	};

	enum LOAD_SHADER_FLAGS
	{
		LS_NONE = 0,
		LS_GEOMETRY = 1,
	};

	struct AtmosphereHash
	{
		uint32_t value[3];

		bool operator==(const AtmosphereHash& r)
		{
			return memcmp(&r, value, sizeof(AtmosphereHash)) == 0;
		}
		void operator=(const AtmosphereHash& r)
		{
			memcpy(&value, &r.value, sizeof(AtmosphereHash));
		}
	};

	struct RenderBuffers
	{
		Texture* color;
		Texture* colorReprojected;
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
		vec4 sun_direction;
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
	Texture* whiteTexture;
	ManagedPtr<Mesh> planeMesh;
	ManagedPtr<Mesh> gridMesh;
	ManagedPtr<Mesh> lineMesh;
	Material* compositeMaterial{};
	Material* finalPostMaterial{};

	struct RenderVector
	{
		vec4 color;
		vec3 v;
	};
	std::vector<RenderVector> renderVectors;

	int specualrQuality{};
	VIEW_MODE viewMode{VIEW_MODE::FINAL};
	bool taa{true};
	bool wireframeAA{false};
	bool wireframe{false};
	ENVIRONMENT_TYPE environmentType{ENVIRONMENT_TYPE::CUBEMAP};
	const int environmentCubemapSize = 256;
	AtmosphereHash atmosphereHash{};
	Texture *environment;
	std::string envirenmentHDRIPath;
	ManagedPtr<Texture> environmentHDRI;
	Texture *environmentAtmosphere;
	float diffuseEnvironemnt{ 1.0f };
	float specularEnvironemnt{ 1.0f };
	const uint64_t maxFrames = 8;
	float gbufferMs;
	float lightsMs;
	float compositeMs;
	float frameMs;

	std::vector<RenderMesh> getRenderMeshes();
	RenderScene getRenderScene();
	void renderGrid();
	void drawMeshes(PASS pass, std::vector<RenderMesh>& meshes);
	void calculateAtmosphereHash(vec4 sun_direction, AtmosphereHash& hash);
public:

	// IProfilerCallback
	uint getNumLines() override { return 4; }
	std::string getString(uint i) override;

public:
	void Init();
	void Update();
	void Free();
	void RenderFrame(size_t viewID, const mat4& ViewMat, const mat4& ProjMat, Model** wireframeModels, int modelsNum);
	auto GetPrevRenderTexture(PREV_TEXTURES id, uint width, uint height, TEXTURE_FORMAT format) -> Texture*;
	void ExchangePrevRenderTexture(Texture *prev, Texture *some);

public:
	auto DLLEXPORT GetShader(const char* path, Mesh* meshattrib = nullptr, const std::vector<std::string>* defines = nullptr, LOAD_SHADER_FLAGS flags = LS_NONE) -> Shader*;
	auto DLLEXPORT GetComputeShader(const char* path, const std::vector<std::string>* defines) -> Shader*;
	auto DLLEXPORT ReloadShaders() -> void;
	auto DLLEXPORT RenderGUI() -> void;
	auto DLLEXPORT DrawMeshes(PASS pass) -> void;
	auto DLLEXPORT GetRenderTexture(uint width, uint height, TEXTURE_FORMAT format, int msaaSamples = 0, TEXTURE_TYPE type = TEXTURE_TYPE::TYPE_2D, bool mips = false) -> Texture*;
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
	auto DLLEXPORT IsWireframeAA() -> bool { return wireframeAA; }
	auto DLLEXPORT SetWireframeAA(bool value) -> void { wireframeAA = value; }
	auto DLLEXPORT IsWireframe() -> bool { return wireframe; }
	auto DLLEXPORT SetWireframe(bool value) -> void { wireframe = value; }
	auto DLLEXPORT SetEnvironmentTexturePath(const char* path) -> void;
	auto DLLEXPORT GetEnvironmentTexturePath() -> const char*;
	auto DLLEXPORT GetEnvironmentType() const -> ENVIRONMENT_TYPE { return static_cast<ENVIRONMENT_TYPE>(environmentType); }
	auto DLLEXPORT SetEnvironmentType(ENVIRONMENT_TYPE type) -> void;
};

