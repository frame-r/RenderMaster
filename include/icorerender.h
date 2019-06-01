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
	auto virtual CreateTexture(uint8 *pData, uint width, uint height, TEXTURE_TYPE type, TEXTURE_FORMAT format, TEXTURE_CREATE_FLAGS flags, int mipmapsPresented) -> ICoreTexture* = 0;
	auto virtual CreateShader(const char *vertText, const char *fragText, const char *geomText, ERROR_COMPILE_SHADER &err) -> ICoreShader* = 0;
	auto virtual CreateStructuredBuffer(uint size, uint elementSize) -> ICoreStructuredBuffer* = 0;

	auto virtual PushStates() -> void = 0;
	auto virtual PopStates() -> void = 0;

	auto virtual SetRenderTextures(int units, Texture **textures, Texture *depthTex) -> void = 0;
	auto virtual Clear() -> void = 0;
	auto virtual SetDepthTest(int enabled) -> void = 0;
	auto virtual SetBlendState(BLEND_FACTOR src, BLEND_FACTOR dest) -> void = 0;
	auto virtual BindTextures(int units, Texture **textures) -> void = 0;
	auto virtual BindStructuredBuffer(int unit, StructuredBuffer *buffer) -> void = 0;
	auto virtual SetMesh(Mesh* mesh) -> void = 0;
	auto virtual SetShader(Shader *shader) -> void = 0;
	auto virtual Draw(Mesh *mesh, uint instances) -> void = 0;
	auto virtual GetViewport(uint* w, uint* h) -> void = 0;
	auto virtual SetViewport(int w, int h) -> void = 0;

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