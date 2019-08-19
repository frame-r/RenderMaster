#pragma once
#include "common.h"
#include "icorerender.h"


class DX11Shader : public ICoreShader
{
	struct SubShader
	{
		ID3D11DeviceChild *pointer;

		// all buffers need to bind for work with shader
		// slot -> index of Constant Buffer in ConstantBufferPool
		std::vector<size_t> _bufferIndicies;
	};

	struct Parameter
	{
		int bufferIndex = -1; // index of ConstantBuffer in ConstantBufferPool
		int parameterIndex = -1; // index in ConstantBuffer::parameters
	};
	std::unordered_map<std::string, Parameter> _parameters; // all shader parameters

	SubShader v{};
	SubShader f{};
	SubShader g{};
	SubShader c{};

	void initSubShader(ShaderInitData& data, SHADER_TYPE type);
	void setParameter(const char *name, const void *data);

public:

	DX11Shader(ShaderInitData& vs, ShaderInitData& fs, ShaderInitData& gs);
	DX11Shader(ShaderInitData& cs);
	virtual ~DX11Shader();

	ID3D11VertexShader*		vs() const { return (ID3D11VertexShader*)v.pointer; }
	ID3D11GeometryShader*	gs() const { return (ID3D11GeometryShader*)g.pointer; }
	ID3D11PixelShader*		fs() const { return (ID3D11PixelShader*)f.pointer; }
	ID3D11ComputeShader*	cs() const { return (ID3D11ComputeShader*)c.pointer; }

	void bind();

	auto virtual SetFloatParameter(const char* name, float value) -> void override;
	auto virtual SetVec4Parameter(const char* name, const vec4 *value) -> void override;
	auto virtual SetMat4Parameter(const char* name, const mat4 *value) -> void override;
	auto virtual SetUintParameter(const char* name, uint value) -> void override;
	auto virtual FlushParameters() -> void override;
};

