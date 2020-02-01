#include "../common.hlsli"
#include "vertex.hlsli"


#ifdef ENG_SHADER_PIXEL

	cbuffer material_parameters
	{
		float4 main_color;
		float4 camera_position;
		float4 base_color;
		float roughness;
		float metalness;
		float normal_intensity;
		float4 albedo_uv;
	};

	#if defined(ENG_INPUT_TEXCOORD) && defined(albedo_map)
		TEXTURE_DECL(albedo, 0)
	#endif
	
	#if defined(ENG_INPUT_TEXCOORD) && defined(ENG_INPUT_NORMAL) && defined(normal_map)
		TEXTURE_DECL(normal, 1)	
	#endif

	#if defined(ENG_INPUT_TEXCOORD) && defined(roughness_map)
		TEXTURE_DECL(roughness, 2)	
	#endif

	#if defined(ENG_INPUT_TEXCOORD) && defined(metalness_map)
		TEXTURE_DECL(metalness, 3)	
	#endif

	struct FS_OUT
	{
		float4 color : SV_Target0;
		float4 shading : SV_Target1;
		float4 normal : SV_Target2;
		float4 velocity : SV_Target3;
	};

	FS_OUT mainFS(VS_OUT fs_input)
	{
		FS_OUT out_color;

		out_color.color = base_color;
		
		out_color.shading.x = roughness;
		#if defined(ENG_INPUT_TEXCOORD) && defined(roughness_map)
			#ifdef is_smoothness
				out_color.shading.x *= (/*roughness*/ srgbInv(TEXTURE_UV_SAMPLE(roughness, fs_input.TexCoord.xy)).r);
			#else
				out_color.shading.x *= (/*smoothness*/ 1.0f - srgbInv(TEXTURE_UV_SAMPLE(roughness, fs_input.TexCoord.xy)).r);
			#endif
		#endif
		
		out_color.shading.y = metalness;
		#if defined(ENG_INPUT_TEXCOORD) && defined(metalness_map)
			out_color.shading.y *= TEXTURE_UV_SAMPLE(metalness, fs_input.TexCoord.xy).r;
		#endif

		#if defined(ENG_INPUT_TEXCOORD) && defined(albedo_map)
			out_color.color *= srgbInv(TEXTURE_UV_SAMPLE(albedo, fs_input.TexCoord.xy));
		#endif

		// velocity
		float2 uv_cur = (fs_input.ViewPos.xy / fs_input.ViewPos.w) * 0.5 + float2(0.5, 0.5);
		float2 uv_prev = (fs_input.ViewPosPrev.xy / fs_input.ViewPosPrev.w) * 0.5 + float2(0.5, 0.5);
		out_color.velocity = float4(uv_cur - uv_prev, 0, 0);

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
