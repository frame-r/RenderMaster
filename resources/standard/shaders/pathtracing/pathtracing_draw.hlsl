#include "pathtracing_common.hlsli"

RWTexture2D<float4> tex : register(u0);

cbuffer SceneBuffer : register(b1)
{
	uint spheresCount;
	uint triCount;
	uint lightsCount;
};

struct Triangle
{
	float4 p0;
	float4 p1;
	float4 p2;
	float4 normal;
	int materialID;
	uint _padding[3];
};
StructuredBuffer<Triangle> triangles : register(t0);

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
StructuredBuffer<AreaLight> lights : register(t1);

#include "pathtracing_intersect.hlsli"

static uint rng_state;
static const float png_01_convert = (1.0f / 4294967296.0f); // to convert into a 01 distribution

//float3 uniformSampleHemisphere(float u1, float u2)
//{
//	float r = sqrt(1.0f - u1 * u1);
//	float phi = _2PI * u2;
//	return float3(cos(phi), sin(phi), u1);
//}
//float3 cosineSampleHemisphere(float u1, float u2)
//{
//	float r = sqrt(u1);
//	float theta = _2PI * u2;
//	return float3(cos(theta), sin(theta), sqrt(max(0, 1 - u1)));
//}

const static float fi = 1.324717957244;
float goldenRatioU1(float seed)
{
	return frac(seed / fi);
}

float goldenRatioU2(float seed)
{
	return frac(seed /(fi * fi));
}

float3 rayUniform(float3 N, float u1, float u2, out float pdf)
{
	float3 UpVector = abs(N.z) < 0.9999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	float z = u1;
	float r = sqrt(max(0.f, 1.0 - z * z));
	float phi = _2PI * u2;
	float x = r * cos(phi);
	float y = r * sin(phi);

	pdf = 1 / (2 * _PI);

	float3 H = normalize(TangentX * x + TangentY * y + N * z);

	return H;
}
float3 rayCosine(float3 N, float u1, float u2, out float pdf)
{
	float3 UpVector = abs(N.z) < 0.9999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	float3 dir;
	float r = sqrt(u1);
	float phi = 2.0 * _PI * u2;
	dir.x = r * cos(phi);
	dir.y = r * sin(phi);
	dir.z = sqrt(max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));

	pdf = dir.z / _PI;

	float3 H = normalize(TangentX * dir.x + TangentY * dir.y + N * dir.z);

	return H;
}

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

// Magic bit shifting algorithm from George Marsaglia's paper
uint rand_xorshift()
{
	rng_state ^= uint(rng_state << 13);
	rng_state ^= uint(rng_state >> 17);
	rng_state ^= uint(rng_state << 5);
	return rng_state;
}

float Uniform01() {
	return float(rand_xorshift() * png_01_convert);
}

void ComputeRngSeed(uint index, uint iteration, uint depth) {
	rng_state = uint(wang_hash((1 << 31) | (depth << 22) | iteration) ^ wang_hash(index));
}

[numthreads(GROUP_DIM_X, GROUP_DIM_Y, 1)]
void mainCS(uint3 dispatchThreadId : SV_DispatchThreadID)
{
#if 1
	const float3 skyColor = float3(0.0, 0.0, 0.0);
#else
	const float3 skyColor = float3(1.0, 1.0, 1.0);
#endif

	uint ii = dispatchThreadId.x;
	uint jj = maxSize_y - dispatchThreadId.y - 1;

	if (ii >= maxSize_x || jj >= maxSize_y)
		return;

	float4 curColor = tex[dispatchThreadId.xy];
	float rays =  curColor.a;

	uint pixelNum = jj * maxSize_x * ii;
	ComputeRngSeed(pixelNum, uint(rays), 0);
	//rng_state = wang_hash(pixel_num);
	
	float2 jitter = float2(Uniform01(), Uniform01());

	float2 ndc;
	ndc.x = (ii + jitter.x) / maxSize_x * 2 - 1;
	ndc.y = (jj + jitter.y) / maxSize_y * 2 - 1;

#if REALTIME==1
	const int bounces = 1;
	const int lightSamples = 1;
#else
	const int bounces = 5;
	const int lightSamples = 1;
#endif
	const float lightSamplesInv = rcp(lightSamples);

	float3 orign = cam_pos_ws.xyz;
	float3 dir = GetWorldRay(ndc, cam_forward_ws.xyz, cam_right_ws.xyz, cam_up_ws.xyz);	
	float3 throughput = float3(1, 1, 1);
	float3 color = 0;

	[unroll]
	for (int i = 0; i < bounces; i++)
	{
		float3 hit, N; int id;
		if (!IntersectWorld(orign, dir, hit, N, id))
		{
			color += skyColor * throughput;
			break;
		}

		orign = hit + N * 0.003;
		
		float3 directLight = 0;
		{
			float tt = lerp(-1, 1, Uniform01());
			float bb = lerp(-1, 1, Uniform01());
			float3 lightSample = lights[0].center + lights[0].T * tt + lights[0].B * bb;

			float3 L = lightSample - hit;
			float L_len = length(L);
			L /= L_len;
		
			float isVisible = 1;
			float3 hitPosition, hitNormal;
			if (IntersectWorld(orign, L, hitPosition, hitNormal, id))
				isVisible = 0;
		
			float brdf = _INVPI;
			float areaLightFactor = max(dot(lights[0].normal, -L), 0)  / (L_len * L_len);
			directLight +=  isVisible * areaLightFactor * brdf * max(dot(L, N), 0);
		}

		color += lights[0].color * (directLight * throughput);
		
		float pdf;
	#if 0
		dir = rayUniform(N, Uniform01(), Uniform01(), pdf);
	#else
		dir = rayCosine(N, Uniform01(), Uniform01(), pdf);
	#endif

		float brdf = _INVPI;
		throughput *= max(dot(dir, N), 0) * brdf / pdf;

	#if 0
		float p = max(throughput.x, max(throughput.y, throughput.z));
		if (Uniform01() > p) {
			break;
		}

		// Add the energy we 'lose' by randomly terminating paths
		throughput *= 1 / p;
	#endif
	}

	color *= _2PI;

	float a = rays / (rays + 1);
	color.rgb = curColor.rgb * a + color.rgb * (1 - a);
	tex[dispatchThreadId.xy] = float4(color.rgb, rays+1);
}
