#include "../common.hlsli"
#include "vertex.hlsli"


#ifdef ENG_SHADER_PIXEL

Texture2D texture_depth : register(t0);

float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
{
	float depth = texture_depth.Load(int3(screenPos.xy, 0)).r;

	for (int i = 0; i < 8; i++)
	{
		float d = texture_depth.Load(int3(screenPos.xy + neigbors_8[i], 0)).r;
		depth = max(d, depth);
	}

	if (fs_input.position.z > depth)
		discard;

	return float4(1, 0, 0, 1);
}

#endif
