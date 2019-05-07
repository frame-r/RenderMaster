
#include "vertex.h"


#ifdef ENG_SHADER_PIXEL

	cbuffer const_buffer_material_parameters
	{
		float4 main_color;
		float4 camera_position;
		float4 color;
		float4 shading;
	};

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

		//float4 ambient = float4(0.05f, 0.05f, 0.05f, 0.0f);
		//float4 diffuse = main_color;
		//float4 specular = float4(0, 0, 0, 0);
		//float roughness = 0.1;
		//
		//#ifdef ENG_INPUT_NORMAL
		//	float4 nL_world = normalize(float4(0.35, -0.45, 2.8, 0));
		//	float3 N = normalize(fs_input.Normal.xyz);
		//	
		//	// diffuse
		//	diffuse = max(dot(N, nL_world.xyz), 0) * main_color;
		//	
		//	// specular
		//	//float3 L = nL_world.xyz;
		//	//float3 V = normalize(camera_position.xyz - fs_input.WorldPosition);
		//	//float3 H = normalize( L + V );
		//	//
		//	//specular = float4(1, 1, 1, 0) * DistributionGGX(N, H, roughness) * max(dot(N, nL_world.xyz), 0);
		//#endif
		//
		//out_color.color = ambient + diffuse /*+ specular*/;
		//return out_color;

		out_color.color = color;
		out_color.shading = shading;

		#ifdef ENG_INPUT_NORMAL
			out_color.normal.xyz = normalize(fs_input.Normal.xyz) * 0.5f + float3(0.5f, 0.5f, 0.5f); // pack normal
			out_color.normal.w = 0;
		#endif

		return out_color;
	}

#endif
