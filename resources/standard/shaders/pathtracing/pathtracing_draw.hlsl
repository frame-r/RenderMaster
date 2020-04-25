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

struct Material
{
	uint type[4];
	float4 albedo;
	float4 shading; // metall, roughness, 0, 0
};
StructuredBuffer<Material> materials : register(t2);

#include "pathtracing_intersect.hlsli"

static uint rng_state;
static const float png_01_convert = (1.0f / 4294967296.0f); // to convert into a 01 distribution

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

float3 directLights(float3 orign, float3 N)
{
	float tt = lerp(-1, 1, Uniform01());
	float bb = lerp(-1, 1, Uniform01());

	float3 directLight = 0;

	for(int i = 0; i < lightsCount; ++i)
	{
		float3 lightSample = lights[i].center + lights[i].T * tt + lights[i].B * bb;

		float3 L = lightSample - orign;
		float L_len = length(L);
		L /= L_len;

		float isVisible = 1;
		float3 hitPosition, hitNormal;
		int id;
		if (IntersectWorld(orign, L, hitPosition, hitNormal, id, L_len))
			isVisible = 0;

		float brdf = _INVPI;
		float areaLightFactor = max(dot(lights[i].normal, -L), 0) / (L_len * L_len);

		directLight += isVisible * areaLightFactor * brdf * max(dot(L, N), 0);
	}

	return directLight;
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
void samplingBRDF(out float3 sampleDir, out float sampleProb, out float3 brdfCos,
				  in float3 surfaceNormal, in float3 baseDir, in uint materialIdx, inout uint seed)
{
	Material mtl = materials[materialIdx];

	float3 brdfEval;
	float3 albedo = mtl.albedo;

	float3 I, O = baseDir, N = surfaceNormal, H;
	float ON = dot(O, N), IN, HN, OH;
	float alpha2 = mtl.shading.y * mtl.shading.y;

#if 0

		sampleDir = rayCosine(N, Uniform01(), Uniform01(), sampleProb);

		//float3 brdf = mat.albedo * _INVPI;
		//throughput *= max(dot(dir, N), 0) * brdf / pdf;
		brdfCos = float3(1,1,1) * _INVPI * max(dot(-sampleDir, N), 0);
#else
	{
		
		H = sample_hemisphere_TrowbridgeReitzCos(alpha2, seed);
		HN = H.z;
		H = applyRotationMappingZToN(N, H);
		OH = dot(O, H);

		I = 2 * OH * H - O;
		IN = dot(I, N);

		if (IN < 0)
		{
			brdfEval = 0;
			sampleProb = 0;		// sampleProb = D*HN / (4*abs(OH));  if allowing sample negative hemisphere
		}
		else
		{
			if (mtl.shading.y >= 0.001f)
			{
				float D = TrowbridgeReitz(HN * HN, alpha2);
				float G = Smith_TrowbridgeReitz(I, O, H, N, alpha2);
				float3 F = albedo + (1 - albedo) * pow(max(0, 1 - OH), 5);
				brdfEval = ((D * G) / (4 * IN * ON)) * F;
				sampleProb = D * HN / (4 * OH);
			}
			else
			{
				brdfEval = albedo / (4 * IN * ON);
				sampleProb = 1 / (4 * OH);		// IN > 0 imply OH > 0
			}
		}
	}

	sampleDir = I;
	brdfCos = brdfEval * IN;

#endif
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
#else
	const int bounces = 5;
#endif

	float3 orign = cam_pos_ws.xyz;
	float3 dir = GetWorldRay(ndc, cam_forward_ws.xyz, cam_right_ws.xyz, cam_up_ws.xyz);	
	float3 throughput = float3(1, 1, 1);
	float3 color = 0;

	[unroll]
	for (int i = 0; i < bounces; i++)
	{
		float3 hit, N;
		int id;
		if (!IntersectWorld(orign, dir, hit, N, id, 1000.0f)) 
		{
			color += skyColor * throughput;
			break;
		}

		if (id < 0) // light
			color += lights[-(id + 1)].color * throughput;

		Material mat = materials[id];

		orign = hit + N * 0.003;

		float3 sampleDir, brdfCos;
		float sampleProb;
		samplingBRDF(sampleDir, sampleProb, brdfCos, N, -dir, id, rng_state);

		throughput *= max(brdfCos / sampleProb,float3(0,0,0));
		dir = sampleDir;

	#if 1
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
