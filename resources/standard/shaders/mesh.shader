
#include "vertex.h"

#define TEXTURE_DECL(NAME, SLOT) \
	Texture2D texture_ ## NAME : register(t ## SLOT); \
	SamplerState sampler_ ## NAME : register(s ## SLOT); \
	cbuffer texture_ ## NAME { \
		float4 uv_transform_ ## NAME; \
	};


#ifdef ENG_SHADER_PIXEL

	cbuffer material_parameters
	{
		float4 main_color;
		float4 camera_position;
		float4 base_color;
		float roughness;
		float metallness;
		float normal_intensity;
		float4 albedo_uv;
	};

	//Texture2D texture_albedo : register(t0);
	//SamplerState sampler_albedo : register(s0);
	//
	//Texture2D texture_normal : register(t1);
	//SamplerState sampler_normal : register(s1);

	TEXTURE_DECL(albedo, 0)
	TEXTURE_DECL(normal, 1)
	

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

		out_color.color = base_color;
		out_color.shading.x = roughness;
		out_color.shading.y = metallness;
		
		#ifdef ENG_INPUT_TEXCOORD
			#ifdef albedo_map
				out_color.color *= texture_albedo.Sample(sampler_albedo, fs_input.TexCoord * uv_transform_albedo.xy + uv_transform_albedo.zw);
			#endif
		#endif

		#ifdef ENG_INPUT_NORMAL
		
			float3 N = normalize(fs_input.Normal.xyz);
			
			#ifdef normal_map
			
				float3 T = cross(ddy(fs_input.WorldPosition.xyz), N) * ddx(fs_input.TexCoord.x) + cross(N, ddx(fs_input.WorldPosition.xyz)) * ddy(fs_input.TexCoord.x);
				T = -T;
				T = normalize(T - N * dot(T, N));
				float3 B = -normalize(cross(N, T));
				float3x3 TBN = float3x3(T, B, N);
				
				float3 normal_ts = texture_normal.Sample(sampler_normal, fs_input.TexCoord.xy * uv_transform_normal.xy + uv_transform_normal.zw) * 2 - float3(1,1,1);
				N = mul(normal_ts, TBN);


				N = lerp(fs_input.Normal.xyz, N, normal_intensity);
			#endif	
	
			out_color.normal.xyz = normalize(N) * 0.5f + float3(0.5f, 0.5f, 0.5f); // pack normal
			out_color.normal.w = 0;
		#endif

		return out_color;
	}

#endif
