#include "common.h"
#include "vertex_post.h"

#ifdef ENG_SHADER_PIXEL

Texture2D texture_color : register(t0);
Texture2D texture_color_prev : register(t1);
Texture2D texture_velocity : register(t2);

cbuffer taa_parameters
{
	float4 viewport_data;
};

static float2 neigbors[8] =
{
	float2( 1, 0),
	float2(-1, 0),
	float2( 0, 1),
	float2( 0,-1),
	float2(-1,-1),
	float2( 1,-1),
	float2( 1, 1),
	float2(-1, 1)
};

float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
{
	float3 color = texture_color.Load(int3(screenPos.xy, 0)).rgb;

	float2 vel = texture_velocity.Load(int3(screenPos.xy, 0)).rg;
	int2 uv_prev = int2(screenPos.xy - vel * (viewport_data.xy ));
	float3 color_prev = texture_color_prev.Load(int3(uv_prev, 0)).rgb;

	float3 color_min = color;
	float3 color_max = color;

	for (int i = 0; i < 4; i++)
	{
		float3 c = texture_color.Load(int3(screenPos.xy + neigbors[i], 0)).rgb;
		color_min = min(c, color_min);
		color_max = max(c, color_max);
	}

	color_prev = clamp(color_prev, color_min, color_max);

	//float lum0 = luma(color);
	//float lum1 = luma(color_prev);
	//float unbiased_diff = abs(lum0 - lum1) / max(lum0, max(lum1, 0.1));
	//float unbiased_weight = 1.0 - unbiased_diff;
	//float unbiased_weight_sqr = unbiased_weight * unbiased_weight;
	//float k_feedback = lerp(0.9, 0.3, saturate(unbiased_weight_sqr));

	float3 coolor_out = lerp(color_prev, color, 0.1);

	return float4(coolor_out, 1);
}

#endif
