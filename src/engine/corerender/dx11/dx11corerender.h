#include "common.h"
#include "icorerender.h"

namespace WRL = Microsoft::WRL;

struct WindowSurface
{
	uint w;
	uint h;
	WRL::ComPtr<IDXGISwapChain> swapChain;
	std::unique_ptr<Texture> color;
	std::unique_ptr<Texture> depth;
};

class DX11CoreRender final : public ICoreRender, IProfilerCallback
{
	WRL::ComPtr<ID3D11Device> _device;
	WRL::ComPtr<ID3D11DeviceContext> _context;

	std::vector<std::function<void()>> _onCleanBroadcast;

	#include "states_pools.inl"

	BlendStatePool _blendStatePool{*this};
	RasterizerStatePool _rasterizerStatePool{*this};
	DepthStencilStatePool _depthStencilStatePool{*this};

	int MSAASamples_{1};
	int VSyncOn_{1};

	WindowSurface *surface{};

	void createCurrentSurface(int w, int h);
	void bindSurface();;
	WRL::ComPtr<ID3DBlob> createShader(ID3D11DeviceChild *&poiterOut, SHADER_TYPE type, const char *src, ERROR_COMPILE_SHADER &err);
	UINT MSAAquality(DXGI_FORMAT format, int MSAASamples);

	struct Timer
	{
		Timer(WRL::ComPtr<ID3D11Query> disjontQuery_) :
			disjontQuery(disjontQuery_)
		{}

		WRL::ComPtr<ID3D11Query> disjontQuery;

		struct TimerPoint
		{
			WRL::ComPtr<ID3D11Query> beginQuery;
			WRL::ComPtr<ID3D11Query> endQuery;
		};

		std::vector<TimerPoint> TimerPoints;
	};
	std::vector<Timer> timers;

	struct Stat
	{
		int drawCalls{0};
		int instances{0};
		size_t triangles{0};
		int clearCalls{0};

		void clear()
		{
			drawCalls = 0;
			instances = 0;
			triangles = 0;
			clearCalls = 0;
		}
	};

public:
	// IProfilerCallback
	uint getNumLines() override;
	std::string getString(uint i) override;

public:
	// Internal API
	void Update() override;

public:
	struct State
	{
		int x = 0, y = 0;
		int width = 0, heigth = 0;

		Shader *shader{};
		Mesh *mesh{};

		ID3D11ShaderResourceView* srvs[16]{};

		ID3D11UnorderedAccessView* uavs[16]{};

		Texture* renderTargets[8]{};
		Texture* renderDepth{};

		D3D11_BLEND_DESC blendStateDesc{};
		WRL::ComPtr<ID3D11BlendState> blendState;

		D3D11_RASTERIZER_DESC rasterStateDesc{};
		WRL::ComPtr<ID3D11RasterizerState> rasterState;

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
		FLOAT depthClearColor = 1.0f;
		UINT8 stencilClearColor = 0;

		FLOAT clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	};

public:
	ID3D11Device* getDevice() { return _device.Get(); }
	ID3D11DeviceContext* getContext() { return _context.Get(); }

	virtual ~DX11CoreRender(){}
	auto Init(const WindowHandle* handle, int MSAASamples, int VSyncOn) -> bool override;
	auto Free() -> void override;
	auto MakeCurrent(const WindowHandle* handle) -> void override;
	auto SwapBuffers() -> void override;
	auto GetSurfaceColorTexture() -> Texture*;
	auto GetSurfaceDepthTexture() -> Texture*;

	auto CreateMesh(const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) -> ICoreMesh* override;
	auto CreateTexture(const uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) -> ICoreTexture* override;
	auto CreateShader(const char *vertText, const char *fragText, const char *geomText, ERROR_COMPILE_SHADER &err) -> ICoreShader* override;
	auto CreateComputeShader(const char *compText, ERROR_COMPILE_SHADER &err) -> ICoreShader* override;
	auto CreateStructuredBuffer(uint size, uint elementSize, BUFFER_USAGE usage) -> ICoreStructuredBuffer* override;

	auto PushStates() -> void override;
	auto PopStates() -> void override;

	auto SetRenderTextures(int units, Texture **textures, Texture *depthTex) -> void override;
	auto Clear() -> void override;
	auto SetDepthTest(int enabled) -> void override;
	auto SetDepthFunc(DEPTH_FUNC func) -> void override;
	auto SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) -> void override;
	auto SetCullingMode(CULLING_MODE value) -> void override;
	auto SetFillingMode(FILLING_MODE value) -> void override;
	auto BindTextures(int units, Texture **textures, BIND_TETURE_FLAGS flags) -> void override;
	auto BindUnorderedAccessTextures(int units, Texture **textures) -> void override;
	auto BindStructuredBuffer(int unit, StructuredBuffer *buffer) -> void override;
	auto SetMesh(Mesh* mesh) -> void override;
	auto SetShader(Shader *shader) -> void override;
	auto Draw(Mesh *mesh, uint instances) -> void override;
	auto Dispatch(uint x, uint y, uint z) -> void override;
	auto GetViewport(uint* w, uint* h) -> void override;
	auto SetViewport(int w, int h, int count = 1) ->void override;
	auto ResizeBuffersByViewort() -> void override;

	auto CreateTimer() -> uint32_t override;
	auto TimersBeginFrame(uint32_t timerID) -> void override;
	auto TimersEndFrame(uint32_t timerID) -> void override;
	auto TimersBeginPoint(uint32_t timerID, uint32_t pointID) -> void override;
	auto TimersEndPoint(uint32_t timerID, uint32_t pointID) -> void override;
	auto GetTimeInMsForPoint(uint32_t timerID, uint32_t pointID) -> float override;

	auto GetName() -> const char * { return "dx11corerender"; }

private:
	State state_{};
	Stat oldStat_;
	Stat stat_;
};
