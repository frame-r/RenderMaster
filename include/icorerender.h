#pragma once
#include "common.h"


class ICoreMesh;
class ICoreShader;
class ICoreTexture;
class ICoreStructuredBuffer;
class Shader;
class Mesh;
class Texture;
class StructuredBuffer;

enum class BIND_TETURE_FLAGS
{
	PIXEL = 1 << 0,
	COMPUTE = 1 << 1
};
DEFINE_ENUM_OPERATORS(BIND_TETURE_FLAGS)

class NOVTABLE ICoreRender
{
public:
	virtual ~ICoreRender() = default;
	// Internal API
	void virtual Update() = 0;

public:
	auto virtual Init(const WindowHandle* handle, int MSAASamples, int VSyncOn) -> bool = 0;
	auto virtual Free() -> void = 0;
	auto virtual MakeCurrent(const WindowHandle* handle) -> void = 0;
	auto virtual SwapBuffers() -> void = 0;
	auto virtual GetSurfaceColorTexture() -> Texture* = 0;
	auto virtual GetSurfaceDepthTexture() -> Texture* = 0;

	auto virtual CreateMesh(const MeshDataDesc *dataDesc, const MeshIndexDesc *indexDesc, VERTEX_TOPOLOGY mode) -> ICoreMesh* = 0;
	auto virtual CreateTexture(const uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) -> ICoreTexture* = 0;
	auto virtual CreateShader(const char *vertText, const char *fragText, const char *geomText, ERROR_COMPILE_SHADER &err) -> ICoreShader* = 0;
	auto virtual CreateComputeShader(const char *compText, ERROR_COMPILE_SHADER &err) -> ICoreShader* = 0;
	auto virtual CreateStructuredBuffer(uint size, uint elementSize, BUFFER_USAGE usage) -> ICoreStructuredBuffer* = 0;

	auto virtual PushStates() -> void = 0;
	auto virtual PopStates() -> void = 0;

	auto virtual SetRenderTextures(int units, Texture **textures, Texture *depthTex) -> void = 0;
	auto virtual Clear() -> void = 0;
	auto virtual SetDepthTest(int enabled) -> void = 0;
	auto virtual SetDepthFunc(DEPTH_FUNC func) -> void = 0;
	auto virtual SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) -> void = 0;
	auto virtual SetCullingMode(CULLING_MODE value) -> void = 0;
	auto virtual SetFillingMode(FILLING_MODE value) -> void = 0;
	auto virtual BindTextures(int units, Texture **textures, BIND_TETURE_FLAGS flags = BIND_TETURE_FLAGS::PIXEL) -> void = 0;
	auto virtual CSBindUnorderedAccessTextures(int units, Texture **textures) -> void = 0;
	auto virtual BindStructuredBuffer(int unit, StructuredBuffer *buffer) -> void = 0;
	auto virtual SetMesh(Mesh* mesh) -> void = 0;
	auto virtual SetShader(Shader *shader) -> void = 0;
	auto virtual Draw(Mesh *mesh, uint instances) -> void = 0;
	auto virtual Dispatch(uint x, uint y, uint z) -> void = 0;
	auto virtual GetViewport(uint* w, uint* h) -> void = 0;
	auto virtual SetViewport(int w, int h, int count = 1) -> void = 0;
	auto virtual ResizeBuffersByViewort() -> void = 0;
	
	auto virtual CreateGPUTiming(uint32_t frames, uint32_t timers)->void;
	auto virtual TimersBeginFrame(uint32_t timerID) -> void;
	auto virtual TimersEndFrame(uint32_t timerID) -> void;
	auto virtual TimersBeginPoint(uint32_t timerID, uint32_t pointID) -> void;
	auto virtual TimersEndPoint(uint32_t timerID, uint32_t pointID) -> void;
	auto virtual GetTimeInMsForPoint(uint32_t timerID, uint32_t pointID) -> float;

	auto virtual GetName() -> const char * = 0;
};

class NOVTABLE ICoreMesh
{
public:
	virtual ~ICoreMesh() = default;
	auto virtual GetNumberOfVertex() -> int = 0;
	auto virtual GetAttributes() -> INPUT_ATTRUBUTE = 0;
	auto virtual GetVertexTopology() -> VERTEX_TOPOLOGY = 0;
	auto virtual GetVideoMemoryUsage() -> size_t = 0;
};

class NOVTABLE ICoreShader
{
public:
	virtual ~ICoreShader() = default;
	auto virtual SetFloatParameter(const char* name, float value) -> void = 0;
	auto virtual SetVec4Parameter(const char* name, const vec4 *value) -> void = 0;
	auto virtual SetMat4Parameter(const char* name, const mat4 *value) -> void = 0;
	auto virtual SetUintParameter(const char* name, uint value) -> void = 0;
	auto virtual FlushParameters() -> void = 0;
};

class NOVTABLE ICoreTexture
{
public:
	virtual ~ICoreTexture() = default;
	auto virtual GetVideoMemoryUsage() -> size_t = 0;
	auto virtual GetWidth() -> int = 0;
	auto virtual GetHeight() -> int = 0;
	auto virtual GetMipmaps() -> int = 0;
	auto virtual ReadPixel2D(void *data, int x, int y) -> int = 0;
	auto virtual GetData(uint8_t* pDataOut, size_t length) -> void = 0;
	auto virtual CreateMipmaps() -> void = 0;
};

class NOVTABLE ICoreStructuredBuffer
{
public:
	virtual ~ICoreStructuredBuffer() = default;
	auto virtual SetData(uint8 *data, size_t size) -> void = 0;
	auto virtual GetSize() -> uint = 0;
	auto virtual GetElementSize() -> uint = 0;
	auto virtual GetVideoMemoryUsage() -> size_t = 0;
};
