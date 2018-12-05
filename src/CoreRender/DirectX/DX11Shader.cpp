#include "pch.h"
#include "DX11Shader.h"
#include "DX11CoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

extern vector<ConstantBuffer> ConstantBufferPool;

ID3D11DeviceContext* getContext(Core *core)
{
	ICoreRender *coreRender = getCoreRender(_pCore);
	DX11CoreRender *dxRender = static_cast<DX11CoreRender*>(coreRender);
	return dxRender->getContext();
}

void DX11Shader::initSubShader(ShaderInitData& data, SHADER_TYPE type)
{
	switch (type)
	{
		case SHADER_TYPE::SHADER_VERTEX: v.pointer.pVertex = (ID3D11VertexShader *)data.pointer; break;
		case SHADER_TYPE::SHADER_GEOMETRY: g.pointer.pGeometry = (ID3D11GeometryShader *)data.pointer; break;
		case SHADER_TYPE::SHADER_FRAGMENT:  f.pointer.pFragment = (ID3D11PixelShader *)data.pointer; break;
	}

	ID3D11ShaderReflection* reflection = nullptr;
	D3D11Reflect(data.bytecode, data.size, &reflection);

	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	// each Constant Buffer
	for(unsigned int i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{			
		ID3D11ShaderReflectionConstantBuffer* buffer = reflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);

		unsigned int registerIndex = 0;
		for(unsigned int k = 0; k < shaderDesc.BoundResources; ++k)
		{
			D3D11_SHADER_INPUT_BIND_DESC ibdesc;
			reflection->GetResourceBindingDesc(k, &ibdesc);

			if(!strcmp(ibdesc.Name, bufferDesc.Name))
				registerIndex = ibdesc.BindPoint;
		}

		vector<ConstantBuffer::ConstantBufferParameter> cbParameters;

		// each parameters
		for (uint j = 0; j < bufferDesc.Variables; j++)
		{
			ID3D11ShaderReflectionVariable *var = buffer->GetVariableByIndex(j);
			D3D11_SHADER_VARIABLE_DESC varDesc;
			var->GetDesc(&varDesc);

			uint bytesVariable = varDesc.Size;

			ConstantBuffer::ConstantBufferParameter p;
			p.name = varDesc.Name;
			p.bytes = varDesc.Size;
			p.offset = varDesc.StartOffset;
			p.elements = 1; // TODO: arrays		

			cbParameters.push_back(p);
		}

		// find existing buffer with same memory layout
		int indexFound = -1;
		{
			for (int j = 0; indexFound == -1 && j < ConstantBufferPool.size(); j++)
			{
				bool isEqual = true;

				if (ConstantBufferPool[j].name != bufferDesc.Name)
					isEqual = false;

				if (ConstantBufferPool[j].bytes != bufferDesc.Size)
					isEqual = false;

				if (ConstantBufferPool[j].parameters.size() != cbParameters.size())
					isEqual = false;

				for (int k = 0; isEqual && k < cbParameters.size(); k++)
				{
					if (ConstantBufferPool[j].parameters[k].name != cbParameters[k].name ||
						ConstantBufferPool[j].parameters[k].bytes != cbParameters[k].bytes ||
						ConstantBufferPool[j].parameters[k].offset != cbParameters[k].offset ||
						ConstantBufferPool[j].parameters[k].elements != cbParameters[k].elements)
					{
						isEqual = 0;
					}
				}

				if (isEqual)
					indexFound = j;
			}
		}

		vector<size_t> *_b;

		switch (type)
		{
			case SHADER_TYPE::SHADER_VERTEX:	_b = &v._bufferIndicies; break;
			case SHADER_TYPE::SHADER_GEOMETRY:	_b = &g._bufferIndicies; break;
			case SHADER_TYPE::SHADER_FRAGMENT:	_b = &f._bufferIndicies; break;
		};

		if (indexFound != -1) // buffer found
		{
			_b->push_back(indexFound);

			for (int i = 0; i < cbParameters.size(); i++)
			{
				_parameters[cbParameters[i].name] = {(int)indexFound, (int)i};
			}
		} else // not found => create new
		{
			WRL::ComPtr<ID3D11Buffer> dxBuffer;
		
			// make byte width multiplied by 16
			uint size = bufferDesc.Size;
			if (size % 16 != 0)
				size = 16 * ((size / 16) + 1);
		
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = size;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = 0;

			ICoreRender *coreRender = getCoreRender(_pCore);
			DX11CoreRender *dxRender = static_cast<DX11CoreRender*>(coreRender);
			auto hr = dxRender->getDevice()->CreateBuffer(&bd, nullptr, dxBuffer.GetAddressOf());		

			_b->push_back(ConstantBufferPool.size());

			for (int k = 0; k < cbParameters.size(); k++)
			{
				_parameters[cbParameters[k].name] = {(int)ConstantBufferPool.size(), (int)k};
			}

			ConstantBufferPool.emplace_back(std::move(ConstantBuffer(dxBuffer, size, bufferDesc.Name, cbParameters)));			
		}
	}
}

DX11Shader::DX11Shader(ShaderInitData& vs, ShaderInitData& fs, ShaderInitData& gs)
{
	initSubShader(vs, SHADER_TYPE::SHADER_VERTEX);
	initSubShader(fs, SHADER_TYPE::SHADER_FRAGMENT);

	if (gs.pointer)
		initSubShader(gs, SHADER_TYPE::SHADER_GEOMETRY);
}

DX11Shader::~DX11Shader()
{
	if (v.pointer.pVertex)		{ v.pointer.pVertex->Release();		v.pointer.pVertex = nullptr; }
	if (f.pointer.pFragment)	{ f.pointer.pFragment->Release();	f.pointer.pFragment = nullptr; }
	if (g.pointer.pGeometry)	{ g.pointer.pGeometry->Release();	g.pointer.pGeometry = nullptr; }
}

void DX11Shader::bind()
{
	ID3D11DeviceContext *ctx = getContext(_pCore);

	ctx->VSSetShader(vs(), nullptr, 0);
	ctx->PSSetShader(fs(), nullptr, 0);

	if (gs())
		ctx->GSSetShader(gs(), nullptr, 0);

	ID3D11Buffer *pointers[128];

	#define BUND_CONSTANT_BUFFERS(PREFIX, IDX_VEC) \
		{ \
			for (size_t i = 0; i < IDX_VEC.size(); i++) \
			{ \
				ConstantBuffer& cb = ConstantBufferPool[IDX_VEC[i]]; \
				pointers[i] = cb.dxBuffer.Get(); \
			} \
			ctx->PREFIX##SetConstantBuffers(0, (uint)IDX_VEC.size(), pointers); \
		}
	
	BUND_CONSTANT_BUFFERS(VS, v._bufferIndicies)
	BUND_CONSTANT_BUFFERS(PS, f._bufferIndicies)

	if (gs())
	BUND_CONSTANT_BUFFERS(GS, g._bufferIndicies)
}

void DX11Shader::setParameter(const char *name, const void *data)
{
	auto it = _parameters.find(name);
	if (it == _parameters.end())
	{
		auto &p = _parameters[name];
		LOG_WARNING_FORMATTED("DX11Shader::setParameter() unable find parameter \"%s\"", name);
	}

	Parameter &p = _parameters[name];

	if (p.bufferIndex < 0 || p.parameterIndex < 0)
		return;

	ConstantBuffer &cb = ConstantBufferPool[p.bufferIndex];
	ConstantBuffer::ConstantBufferParameter &pCB = cb.parameters[p.parameterIndex];
	uint8 *pointer = cb.data.get() + pCB.offset;
	if (memcmp(pointer, data, pCB.bytes))
	{
		memcpy(pointer, data, pCB.bytes);
		cb.needFlush = true;
	}
}

API DX11Shader::SetFloatParameter(const char *name, float value)
{
	setParameter(name, &value);
	return S_OK;
}

API DX11Shader::SetVec4Parameter(const char *name, const vec4 *value)
{
	setParameter(name, value);
	return S_OK;
}

API DX11Shader::SetMat4Parameter(const char *name, const mat4 *value)
{
	setParameter(name, value);
	return S_OK;
}

API DX11Shader::SetUintParameter(const char *name, uint value)
{
	setParameter(name, &value);
	return S_OK;
}

API DX11Shader::FlushParameters()
{
	ID3D11DeviceContext *ctx = getContext(_pCore);

	auto updateBuffers = [ctx](vector<size_t>& indicies)
	{
		for (size_t &idx : indicies)
		{
			ConstantBuffer& cb = ConstantBufferPool[idx];
			if (cb.needFlush)
			{
				ctx->UpdateSubresource(cb.dxBuffer.Get(), 0, nullptr, cb.data.get(), 0, 0);
				cb.needFlush = false;
			}
		}
	};

	updateBuffers(v._bufferIndicies);
	updateBuffers(f._bufferIndicies);

	if (gs())
		updateBuffers(g._bufferIndicies);

	return S_OK;
}


