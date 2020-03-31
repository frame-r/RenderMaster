#include "../common.hlsli"

struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 WorldPosition : TEXCOORD1;
	float4 ViewPos : TEXCOORD2;
#ifdef ENG_INPUT_NORMAL
	float4 Normal : TEXCOORD3;
#endif
#ifdef ENG_INPUT_TEXCOORD
	float2 TexCoord : TEXCOORD4;
#endif
#ifdef ENG_INPUT_COLOR
	float4 Color : TEXCOORD5;
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

	vs_output.ViewPos = mul(MVP, vs_input.PositionIn);

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


#ifdef ENG_SHADER_PIXEL

	cbuffer material_parameters
	{
		float4 sun_dir;
	};

	struct FS_OUT
	{
		float4 color : SV_Target0;
	};

	FS_OUT mainFS(VS_OUT fs_input)
	{
		FS_OUT out_color;
		out_color.color = max(dot(fs_input.Normal.xyz, sun_dir.xyz), 0);
		return out_color;
	}

#endif
