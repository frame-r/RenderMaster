struct UBO final
{
	string name;
	size_t bytes = 0u;
	bool needFlush = true;
	GLuint ID = 0u;
	unique_ptr<uint8[]> data;

	struct Parameter
	{
		string name;
		size_t offset = 0u;
		size_t bytes = 0u;
		size_t elements = 1u; // number of elements in array (if parameter is array)
	};
	vector<Parameter> parameters;

public:

	UBO(GLuint IDIn, size_t bytesIn, const string& nameIn, const vector<Parameter>& paramsIn) :
		ID(IDIn), bytes(bytesIn), name(nameIn), parameters(paramsIn)
	{
		data = std::make_unique<uint8[]>(bytesIn);
		memset(data.get(), 0, bytesIn);
	}

	UBO(const UBO& r) = delete;

	UBO(UBO&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		ID = r.ID;
		r.ID = 0u;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}

	UBO& operator=(UBO&& r)
	{
		name = r.name;
		bytes = r.bytes;
		parameters = std::move(r.parameters);
		ID = r.ID;
		r.ID = 0u;
		data = std::move(r.data);
		needFlush = r.needFlush;
	}

	UBO& operator=(const UBO& r) = delete;

	~UBO()
	{
		if (ID) { glDeleteBuffers(1, &ID);
		ID = 0u; }
	}
};

class GLRenderTarget final : public ICoreRenderTarget
{
	GLuint _ID = 0u;
	GLuint _colors[8];
	GLuint _depth = 0u;

public:
	GLRenderTarget(GLuint idIn);
	virtual ~GLRenderTarget(){ if (_ID) glDeleteFramebuffers(1, &_ID); _ID = 0; }

	GLuint ID() { return _ID; }

	void bind();

	API_RESULT SetColorTexture(uint slot, ITexture *tex) override;
	API_RESULT SetDepthTexture(ITexture *tex) override;
	API_RESULT UnbindColorTexture(uint slot) override;
	API_RESULT UnbindAll() override;
};
