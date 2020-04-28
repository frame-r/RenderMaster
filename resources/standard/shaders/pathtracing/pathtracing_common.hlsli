#pragma once

#include "../common.hlsli"

#define _PI (3.1415926f)
#define _2PI (2.0f * _PI)
#define _INV2PI (rcp(_2PI))
#define _INVPI (rcp(_PI))

struct Triangle
{
	float4 p0;
	float4 p1;
	float4 p2;
	float4 normal;
	int materialID;
	uint _padding[3];
};

struct AreaLight
{
	float4 p0;
	float4 p1;
	float4 p2;
	float4 p3;
	float4 center;
	float4 normal;
	float4 T, B;
	float4 color;
};

struct Material
{
	uint type[4];
	float4 albedo;
	float4 shading; // metall, roughness, reflectivity, 0
};

static const float fi = 1.324717957244;
static uint rng_state;
static const float png_01_convert = (1.0f / 4294967296.0f); // to convert into a 01 distribution

// Wang hash for randomizing
uint wang_hash(uint seed)
{
	seed = (seed ^ 61) ^ (seed >> 16);
	seed *= 9;
	seed = seed ^ (seed >> 4);
	seed *= 0x27d4eb2d;
	seed = seed ^ (seed >> 15);
	return seed;
}

uint rand_xorshift()
{
	rng_state ^= uint(rng_state << 13);
	rng_state ^= uint(rng_state >> 17);
	rng_state ^= uint(rng_state << 5);
	return rng_state;
}

float Uniform01()
{
	return float(rand_xorshift() * png_01_convert);
}

void ComputeRngSeed(uint index, uint iteration, uint depth)
{
	rng_state = uint(wang_hash((1 << 31) /*| (depth << 22)*/ | iteration) ^ wang_hash(index));
}

float goldenRatioU1(float seed)
{
	return frac(seed / fi);
}

float goldenRatioU2(float seed)
{
	return frac(seed / (fi * fi));
}



float3 applyRotationMappingZToN(in float3 N, in float3 v)	// --> https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d
{
	float  s = (N.z >= 0.0f) ? 1.0f : -1.0f;
	v.z *= s;

	float3 h = float3(N.x, N.y, N.z + s);
	float  k = dot(v, h) / (1.0f + abs(N.z));

	return k * h - v;
}

float Smith_TrowbridgeReitz(in float3 wi, in float3 wo, in float3 wm, in float3 wn, in float alpha2)
{
	if (dot(wo, wm) < 0 || dot(wi, wm) < 0)
		return 0.0f;

	float cos2 = dot(wn, wo);
	cos2 *= cos2;
	float lambda1 = 0.5 * (-1 + sqrt(1 + alpha2 * (1 - cos2) / cos2));
	cos2 = dot(wn, wi);
	cos2 *= cos2;
	float lambda2 = 0.5 * (-1 + sqrt(1 + alpha2 * (1 - cos2) / cos2));
	return 1 / (1 + lambda1 + lambda2);
}

float rnd(inout uint seed)
{
	seed = (1664525u * seed + 1013904223u);
	return ((float)(seed & 0x00FFFFFF) / (float)0x01000000);
}

float3 sample_hemisphere_TrowbridgeReitzCos(in float alpha2, inout uint seed)
{
	float3 sampleDir;

	float u = rnd(seed);
	float v = rnd(seed);

	float tan2theta = alpha2 * (u / (1 - u));
	float cos2theta = 1 / (1 + tan2theta);
	float sinTheta = sqrt(1 - cos2theta);
	float phi = _2PI * v;

	sampleDir.x = sinTheta * cos(phi);
	sampleDir.y = sinTheta * sin(phi);
	sampleDir.z = sqrt(cos2theta);

	return sampleDir;
}

float TrowbridgeReitz(in float cos2, in float alpha2)
{
	float x = alpha2 + (1 - cos2) / cos2;
	return alpha2 / (_PI * cos2 * cos2 * x * x);
}

float3 sample_hemisphere_cos(inout uint seed)
{
	float3 sampleDir;

	float param1 = rnd(seed);
	float param2 = rnd(seed);

	// Uniformly sample disk.
	float r = sqrt(param1);
	float phi = 2.0f * _PI * param2;
	sampleDir.x = r * cos(phi);
	sampleDir.y = r * sin(phi);

	// Project up to hemisphere.
	sampleDir.z = sqrt(max(0.0f, 1.0f - r * r));

	return sampleDir;
}
