#pragma once

#include "../common.hlsli"

cbuffer CameraBuffer : register(b0)
{
	float4 cam_forward_ws;
	float4 cam_right_ws;
	float4 cam_up_ws;
	float4 cam_pos_ws;
	uint4 maxSize;
};

#define _PI (3.1415926f)
#define _2PI (2.0f * _PI)
#define _INV2PI (rcp(_2PI))
#define _INVPI (rcp(_PI))

float rand(float2 co) {
	return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

//float radicalInverseVdC(uint bits)
//{
//	bits = (bits << 16) | (bits >> 16);
//	bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
//	bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
//	bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
//	bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
//	return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
//}
//
//float2 hammersley2d(int i, int N)
//{
//	return float2(float(i) / float(N), radicalInverseVdC(i));
//}

//float luma(float3 col)
//{
//	return 0.299 * col.r + 0.587 * col.g + 0.114 * col.b;
//}
//float3 srgb(float3 v)
//{
//	return float3(pow(abs(v.x), 0.45), pow(abs(v.y), 0.45), pow(abs(v.z), 0.45));
//}
//float4 srgb(float4 v)
//{
//	return float4(pow(abs(v.x), 0.45), pow(abs(v.y), 0.45), pow(abs(v.z), 0.45), pow(abs(v.w), 0.45));
//}
//float3 srgbInv(float3 v)
//{
//	return float3(pow(v.x, 2.2), pow(v.y, 2.2), pow(v.z, 2.2));
//}
//float4 srgbInv(float4 v)
//{
//	return float4(pow(v.x, 2.2), pow(v.y, 2.2), pow(v.z, 2.2), pow(v.w, 2.2));
//}

//
// Tonemapping
//
//float3 tonemapReinhard(float3 x)
//{
//	return x / (1.0 + luma(x));
//}

// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
//float3 tonemapUnreal(float3 x)
//{
//	return x / (x + 0.155) * 1.019;
//}
//
//float3 tonemapACES(float3 x)
//{
//	const float a = 2.51;
//	const float b = 0.03;
//	const float c = 2.43;
//	const float d = 0.59;
//	const float e = 0.14;
//	return (x * (a * x + b)) / (x * (c * x + d) + e);
//}
