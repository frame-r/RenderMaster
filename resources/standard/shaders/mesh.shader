
#include "vertex.h"


#ifdef ENG_SHADER_PIXEL

	cbuffer const_buffer_material_parameters
	{
		float4 main_color;
		float4 camera_position;
		float4 color;
		float4 shading;
		float4 albedo_uv;
	};


	Texture2D texture_albedo : register(t0);
	SamplerState sampler_albedo : register(s0);	

	struct FS_OUT
	{
		float4 color : SV_Target0;
		float4 shading : SV_Target1;

		#ifdef ENG_INPUT_NORMAL
			float4 normal : SV_Target2;
		#endif
	};

	FS_OUT mainFS(VS_OUT fs_input)
	{
		FS_OUT out_color;

		out_color.color = color;
		#ifdef ENG_INPUT_TEXCOORD
			out_color.color *= texture_albedo.Sample(sampler_albedo, fs_input.TexCoord * albedo_uv.xy + albedo_uv.zw);
		#endif

		out_color.shading = shading;

		#ifdef ENG_INPUT_NORMAL
			out_color.normal.xyz = normalize(fs_input.Normal.xyz) * 0.5f + float3(0.5f, 0.5f, 0.5f); // pack normal
			out_color.normal.w = 0;
		#endif

		return out_color;
	}

#endif
