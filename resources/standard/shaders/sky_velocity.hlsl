#include "common.hlsli"
#include "vertex_post.hlsli"

#ifdef ENG_SHADER_PIXEL

cbuffer cam_parameters
{
	float4x4 VP_prev;
	float4x4 VP_inv;
};

float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
{
	float4 position = mul(VP_inv, float4(fs_input.ndc, 1.0f, 1.0f));
	float3 V_ws = normalize(position.xyz / position.w);
	
	float4 p_prev = mul(VP_prev, float4(V_ws, 0));
	float2 ndc_prev = p_prev.xy / p_prev.w;	

	return float4(0.5 * (fs_input.ndc - ndc_prev), 0, 1);
}

#endif
