#include "common.hlsli"
#include "atmosphere.hlsli"

struct VS_OUT
{
	float4 position : SV_Position;
	uint instanceId : TEXCOORD1;
	float3 view : TEXCOORD2;
};

struct GS_OUT
{
	float4 pos : SV_Position;
	uint slice : SV_RenderTargetArrayIndex;
	float3 view : TEXCOORD1;
};

static float3x3 ViewSliceTransform[6] =
{
	float3x3(float3(0, 0, 1), float3(0, -1, 0), float3(1, 0, 0)),
	float3x3(float3(0, 0, -1), float3(0, -1, 0), float3(-1, 0, 0)),
	float3x3(float3(1, 0, 0), float3(0, 0, -1), float3(0, 1, 0)),
	float3x3(float3(1, 0, 0), float3(0, 0, 1), float3(0, -1, 0)),
	float3x3(float3(1, 0, 0), float3(0, -1, 0), float3(0, 0, -1)),
	float3x3(float3(-1, 0, 0), float3(0, -1, 0), float3(0, 0, 1))
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
		uint instanceId : SV_InstanceID;
	};

	VS_OUT mainVS(VS_IN vs_input)
	{
		VS_OUT vs_output;
		vs_output.position.xy = vs_input.PositionIn.xy;
		vs_output.position.zw = float2(0, 1);
		vs_output.view = mul(ViewSliceTransform[vs_input.instanceId], normalize(vs_output.position.xyw));
		vs_output.instanceId = vs_input.instanceId;
		return vs_output;
	}
#endif

#ifdef ENG_SHADER_GEOMETRY
	[maxvertexcount(3)]
	void mainGS(triangle VS_OUT input[3], inout TriangleStream<GS_OUT> OutputStream)
	{
		for (int i = 0; i < 3; i++)
		{
			GS_OUT o;
			o.pos = input[i].position;
			o.slice = input[i].instanceId;
			o.view = input[i].view;
			OutputStream.Append(o);
		}
	}
#endif

#ifdef ENG_SHADER_PIXEL

	cbuffer sun_parameters
	{
		float4 sun_direction;
	};

	TextureCube texture_environment : register(t0);
	SamplerState sampler_environment : register(s0);

	float4 mainFS(GS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target
	{
		float3 V = float3(fs_input.view.x, -fs_input.view.y, -fs_input.view.z);

		float3 color = get_atm(V.xzy, sun_direction.xzy);

		return float4(color.rgb, 1);
	}

#endif
