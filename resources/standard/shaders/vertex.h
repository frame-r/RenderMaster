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
		float4x4 M;
		float4x4 NM;
	};

	struct VS_IN
	{
		float4 PositionIn : POSITION0;
		#ifdef ENG_INPUT_NORMAL
			float4 NormalIn : TEXCOORD1;
		#endif
		#ifdef ENG_INPUT_TEXCOORD
			float2 TexCoordIn : TEXCOORD2;
		#endif
		#ifdef ENG_INPUT_COLOR
			float4 ColorIn : TEXCOORD3;
		#endif
	};

	VS_OUT mainVS(VS_IN vs_input)
	{
		VS_OUT vs_output;
		vs_output.position = mul(MVP, vs_input.PositionIn);

		vs_output.WorldPosition = mul(M, vs_input.PositionIn);

		#ifdef ENG_INPUT_NORMAL
			vs_output.Normal = normalize(mul(NM, vs_input.NormalIn));
		#endif

		#ifdef ENG_INPUT_TEXCOORD
			vs_output.TexCoord = vs_input.TexCoordIn;
		#endif

		#ifdef ENG_INPUT_COLOR
			vs_output.Color = vs_input.ColorIn;
		#endif

		return vs_output;
	}
#endif
