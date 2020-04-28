#include "pathtracing_common.hlsli"

cbuffer GlobalBuffer : register(b0)
{
	float4 camForwardWS;
	float4 camRightWS;
	float4 camUpWS;
	float4 camPosWS;
	uint maxSizeX;
	uint maxSizeY;
	uint spheresCount;
	uint triCount;
	uint lightsCount;
};

RWTexture2D<float4> tex : register(u0);
StructuredBuffer<Triangle> triangles : register(t0);
StructuredBuffer<AreaLight> lights : register(t1);
StructuredBuffer<Material> materials : register(t2);

#include "pathtracing_intersect.hlsli"

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


void samplingBRDF(out float3 sampleDir, out float sampleProb, out float3 brdfCos,
				  in float3 surfaceNormal, in float3 baseDir, in uint materialIdx, inout uint seed)
{
	Material mtl = materials[materialIdx];

	float3 brdfEval;
	float3 albedo = mtl.albedo;

	float3 I, O = baseDir, N = surfaceNormal, H;
	float ON = dot(O, N), IN, HN, OH;
	float alpha2 = mtl.shading.y * mtl.shading.y;
	float reflectivity = mtl.shading.z;
	float r = reflectivity;
	float metallic =  mtl.shading.x;
	float Rd = (1 - metallic);
	float diffuseRatio = 0.5f * (1.0f - metallic);

	if (rnd(seed) > diffuseRatio)
	{
		H = sample_hemisphere_TrowbridgeReitzCos(alpha2, seed);
		HN = H.z;
		H = applyRotationMappingZToN(N, H);
		OH = dot(O, H);

		I = 2 * OH * H - O;
		IN = dot(I, N);
	}
	else
	{
		I = sample_hemisphere_cos(seed);
		IN = I.z;
		I = applyRotationMappingZToN(N, I);

		H = O + I;
		H = (1 / length(H)) * H;
		HN = dot(H, N);
		OH = dot(O, H);
	}

	[flatten]
	if (IN < 0)
	{
		brdfEval = 0;
		sampleProb = 0;
	}
	else
	{
		float3 F0 = float3(.2, .2, .2) * reflectivity;
		F0 = lerp(F0, albedo, metallic);

		float3 F = fresnelSchlick(OH, F0);
		float D = TrowbridgeReitz(HN * HN, max(alpha2, .0005f));
		float G = Smith_TrowbridgeReitz(I, O, H, N, alpha2);
		float3 spec = ((D * G) / (4 * IN * ON)) * F;

		float3 diff =  (28.f / (23.f * _PI)) * Rd * 
			(1 - pow(1 - .5 * ON, 5)) *
			(1 - pow(1 - .5 * IN, 5)) * (float3(1,1,1) - F0) * albedo;

		brdfEval = spec + diff;

		sampleProb = .5 * (D * HN / (4 * OH) + (_INVPI * IN));
	}

	sampleDir = I;
	brdfCos = brdfEval * IN;

}

[numthreads(GROUP_DIM_X, GROUP_DIM_Y, 1)]
void mainCS(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	const float3 skyColor = float3(0.0, 0.0, 0.0);
	const int bounces = 5;
	const int iterations = 5;

	uint ii = dispatchThreadId.x;
	uint jj = maxSizeY - dispatchThreadId.y - 1;

	float4 curColor = tex[dispatchThreadId.xy];
	float rays =  curColor.a;
	uint raysNum = uint(rays);
	uint pixelNum = jj * maxSizeX + ii;
	ComputeRngSeed(pixelNum, uint(rays), 0);
	
	float2 jitter = float2(Uniform01(), Uniform01()) - float2(.5, .5);
	float2 ndc = float2(
		float(ii + jitter.x) / maxSizeX * 2 - 1,
		float(jj + jitter.y) / maxSizeY * 2 - 1);

	float3 color = 0;

	for (int l = 0; l < iterations; l++)
	{
		float3 orign = camPosWS.xyz;
		float3 dir = GetWorldRay(ndc, camForwardWS.xyz, camRightWS.xyz, camUpWS.xyz);	
		float3 throughput = float3(1, 1, 1);

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

			throughput *= max(brdfCos / sampleProb, float3(0,0,0));
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
	}

	color /= iterations;

	color *= _2PI;

	float a = rays / (rays + 1);
	color.rgb = curColor.rgb * a + color.rgb * (1 - a);

	if (ii < maxSizeX && jj < maxSizeY)
		tex[dispatchThreadId.xy] = float4(color.rgb, rays+1);
}
