
#include "common.hlsli"

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
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
		uint instance : SV_INSTANCEID;
	};

	cbuffer viewport_parameters
	{
		float invWidth2;
		float invHeight2;
	};

	struct Char {
		float4 data;
		uint id;
		uint dummy__[3];
	};
	StructuredBuffer<Char> character_buffer : register(t1);

	VS_OUT mainVS(VS_IN vs_input)
	{
		VS_OUT vs_output;
		const uint left_padding = 10;
		const uint top_padding = 32;
		const float ww = 32.0f;
		const float hh = 32.0f;
		const uint chars_in_row = 16u;
		const float tex_size = 512.0f;
		
		float w = character_buffer[vs_input.instance].data.x;
		float offset = character_buffer[vs_input.instance].data.y;
		float offsetVert = character_buffer[vs_input.instance].data.z;

		float2 vtx = vs_input.PositionIn.xy * 0.5f + float2(0.5f, 0.5f); // [-1, 1] -> [0, 1]

		float2 ndc = 
			float2(-1, 1) +
			float2(invWidth2 * left_padding, -invHeight2 * top_padding) +
			float2(invWidth2 * offset, invHeight2 * offsetVert) + 
			float2(invWidth2 * w, invHeight2 * hh) * vtx;
		
		vs_output.position = float4(ndc, 0.0f, 1.0f);

		vtx.y = 1.0f - vtx.y;

		uint id = character_buffer[vs_input.instance].id - 32;
		float uv_x = float(id % chars_in_row) * ww;
		float uv_y = float(id / chars_in_row) * hh;

		uv_x += vtx.x * w;
		uv_y += vtx.y * hh;

		uv_x = uv_x / tex_size;
		uv_y = uv_y / tex_size;

		vs_output.TexCoord = float2(uv_x, uv_y);
		
		return vs_output;
	}

#elif ENG_SHADER_PIXEL

	Texture2D texture_font : register(t0);
	SamplerState sampler_font : register(s0);

	float4 mainFS(VS_OUT fs_input) : SV_Target0
	{
		float3 tex = texture_font.Sample(sampler_font, fs_input.TexCoord).rgb;

		float4 color = float4(float3(1, 1, 1) * tex, luma(tex));

		return color;
	}

#endif
