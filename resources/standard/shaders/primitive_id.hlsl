
struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 WorldPosition : TEXCOORD1;
	#ifdef ENG_INPUT_NORMAL
		float4 Normal : TEXCOORD2;
	#endif
	#ifdef ENG_INPUT_TEXCOORD
		float2 TexCoord : TEXCOORD3;
	#endif
	#ifdef ENG_INPUT_COLOR
		float4 Color : TEXCOORD4;
	#endif
};
	
#ifdef ENG_SHADER_VERTEX

	cbuffer vertex_transformation_parameters
	{
		float4x4 MVP;
	};

	struct VS_IN
	{
		float4 PositionIn : POSITION0;
	};

	VS_OUT mainVS(VS_IN vs_input)
	{
		VS_OUT vs_output;
		vs_output.position = mul(MVP, vs_input.PositionIn);

		return vs_output;
	}

#elif ENG_SHADER_PIXEL

	cbuffer const_buffer_model_parameters
	{
		uint id;
	};

	uint mainFS(VS_OUT fs_input) : SV_Target0
	{
		return id;
	}

#endif
