#include "common.hlsli"
#include "vertex_post.hlsli"

#ifdef ENG_SHADER_PIXEL

Texture2DMS<float4> texture_in : register(t0);

float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
{
	float4 col = float4(0,0,0,0);

	for(int i = 0; i < 4; i++)
		col += texture_in.Load(screenPos.xy, i) * 0.25;

	return col;
}

#endif
