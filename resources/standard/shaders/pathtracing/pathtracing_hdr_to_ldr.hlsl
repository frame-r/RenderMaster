#include "../common.hlsli"
#include "../vertex_post.hlsli"


#ifdef ENG_SHADER_PIXEL
	#include "common.hlsli"

	cbuffer PostprocessingBuffer : register(b0)
	{
		float4 data; // x-exposure
	};

	Texture2D color_tex : register(t0);

	float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
	{
		return color_tex.Load(int3(screenPos.xy, 0));

		//color *= data.x; // exposure
		//
		//#if TM_ACES==1
		//	color.rgb = tonemapACES(color.rgb);
		//#endif
		//
		//#if TM_UE==1
		//	color.rgb = tonemapUnreal(color.rgb);
		//#endif
		//
		//#if TM_Reinhard==1
		//	color.rgb = tonemapReinhard(color.rgb);
		//#endif
		//
		//#if TM_UE!=1 
		//	color = srgb(color);
		//#endif

		//return color;
	}

#endif
