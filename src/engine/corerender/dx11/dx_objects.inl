#pragma once

struct ConstantBuffer
{
	string name;
	size_t bytes = 0u;
	bool needFlush = true;
	WRL::ComPtr<ID3D11Buffer> buffer;
	std::unique_ptr<uint8[]> data;

	struct Parameter
	{
		string name;
		size_t offset;
		size_t bytes;
	};
	vector<Parameter> parameters;

public:
	ConstantBuffer(WRL::ComPtr<ID3D11Buffer> dxBufferIn, int bytesIn, const string& nameIn, const vector<Parameter>& paramsIn) :
		buffer(dxBufferIn), bytes(bytesIn), name(nameIn), parameters(paramsIn)
	{
		data = std::make_unique<uint8[]>(bytesIn);
		memset(data.get(), 0, bytesIn);
	}

	ConstantBuffer(const ConstantBuffer& r) = delete;
	ConstantBuffer& operator=(const ConstantBuffer& r) = delete;
	
	ConstantBuffer(ConstantBuffer&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		buffer = r.buffer;
		r.buffer = nullptr;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}
	ConstantBuffer& operator=(ConstantBuffer&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		buffer = r.buffer;
		r.buffer = nullptr;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}
};
