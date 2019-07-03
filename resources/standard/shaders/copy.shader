#include "common.h"
#include "vertex_post.h"

#ifdef ENG_SHADER_PIXEL

Texture2D texture_albedo : register(t0);
Texture2D texture_normals : register(t1);
Texture2D texture_shading : register(t2);
Texture2D texture_diffuse_light : register(t3);
Texture2D texture_specular_light : register(t4);
Texture2D texture_velocity : register(t5);
Texture2D texture_color : register(t6);
Texture2D texture_color_reprojection : register(t7);

float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
{

	#ifdef VIEW_MODE_NORMAL
		float3 N = texture_normals.Load(int3(screenPos.xy, 0)).rgb * 2.0 - float3(1, 1, 1);
		return float4(N.x * 0.5 + 0.5, N.y * 0.5 + 0.5, N.z * 0.5 + 0.5, 1.0);
	#elif VIEW_MODE_ALBEDO
		return texture_albedo.Load(int3(screenPos.xy, 0));
	#elif VIEW_MODE_DIFFUSE_LIGHT
		return float4(1,1,1, 1.0); // not impl
	#elif VIEW_MODE_SPECULAR_LIGHT
		return float4(1,1,1, 1.0); // not impl
	#elif VIEW_MODE_VELOCITY
		return texture_velocity.Load(int3(screenPos.xy, 0)) * 10;
	#elif VIEW_MODE_COLOR_REPROJECTION
		return texture_color_reprojection.Load(int3(screenPos.xy, 0));
	#else
		return texture_color.Load(int3(screenPos.xy, 0));
	#endif
}

#endif
