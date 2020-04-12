#include "../common.hlsli"

RWTexture2D<float4> tex;

cbuffer CameraBuffer : register(b0)
{
	uint maxSize_x;
	uint maxSize_y;
};

[numthreads(GROUP_DIM_X, GROUP_DIM_Y, 1)]
void mainCS(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	if (dispatchThreadId.x < maxSize_x && dispatchThreadId.y < maxSize_y)
		tex[dispatchThreadId.xy] = float4(0, 0, 0, 0);
}
