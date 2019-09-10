#include "vertex.hlsli"


#ifdef ENG_SHADER_PIXEL

float4 mainFS(VS_OUT fs_input) : SV_Target0
{
	return float4(0.5, 0, 0, 0.3);
}

#endif
