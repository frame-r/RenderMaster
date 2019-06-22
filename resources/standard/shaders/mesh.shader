#include "common.h"
#include "vertex.h"


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

	#if defined(ENG_INPUT_TEXCOORD) && defined(albedo_map)
		TEXTURE_DECL(albedo, 0)
	#endif
	
	#if defined(ENG_INPUT_TEXCOORD) && defined(ENG_INPUT_NORMAL) && defined(albedo_map)
		TEXTURE_DECL(normal, 1)	
	#endif

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
		
		#if defined(ENG_INPUT_TEXCOORD) && defined(albedo_map)
			out_color.color *= TEXTURE_UV_SAMPLE(albedo, fs_input.TexCoord.xy);
		#endif

		#ifdef ENG_INPUT_NORMAL
		
			float3 N = normalize(fs_input.Normal.xyz);
			out_color.normal.xyz = normalize(N) * 0.5f + float3(0.5f, 0.5f, 0.5f); // pack normal
			out_color.normal.w = 0;
			
			#if defined(ENG_INPUT_TEXCOORD) && defined(normal_map) 
				float3 T = -cross(ddy(fs_input.WorldPosition.xyz), N) * ddx(fs_input.TexCoord.x) - cross(N, ddx(fs_input.WorldPosition.xyz)) * ddy(fs_input.TexCoord.x);
				T = normalize(T - N * dot(T, N));
				float3 B = normalize(cross(T, N));
				
				float3x3 TBN = float3x3(T, B, N);
				
				float3 normal_ts = TEXTURE_UV_SAMPLE(normal, fs_input.TexCoord.xy) * 2 - float3(1, 1, 1);
				float3 normal_ws = mul(normal_ts, TBN);

				N = lerp(fs_input.Normal.xyz, normal_ws, normal_intensity);	
	
				out_color.normal.xyz = normalize(N) * 0.5f + float3(0.5f, 0.5f, 0.5f); // pack normal
			#endif
		#endif

		return out_color;
	}

#endif
