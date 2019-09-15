#include "common.hlsli"
#include "vertex_post.hlsli"

#ifdef ENG_SHADER_PIXEL

Texture2D texture_prev : register(t0);
Texture2D texture_velocity : register(t1);

cbuffer cam_parameters
{
	float4 bufer_size;
};

float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
{
	float2 vel = texture_velocity.Load(int3(screenPos.xy, 0)).rg;

	int2 uv = int2(float2(screenPos.xy) - vel * float2(bufer_size.x, -bufer_size.y));

	return  texture_prev.Load(int3(uv, 0));
}

#endif
