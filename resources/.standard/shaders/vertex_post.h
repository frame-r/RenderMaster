struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD1;
	float2 ndc : TEXCOORD2;
};
	
#ifdef ENG_SHADER_VERTEX

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
		vs_output.position.xy = vs_input.PositionIn.xy;
		vs_output.ndc = vs_input.PositionIn.xy;
		vs_output.position.zw = float2(0, 1);
		vs_output.uv = vs_input.PositionIn.xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
		return vs_output;
	}
#endif
