

static const float Reflectance = 0.4;

#define TEXTURE_DECL(NAME, SLOT) \
	Texture2D texture_ ## NAME : register(t ## SLOT); \
	SamplerState sampler_ ## NAME : register(s ## SLOT); \
	cbuffer texture_ ## NAME { \
		float4 uv_transform_ ## NAME; \
	};
	
#define TEXTURE_UV_SAMPLE(NAME, UV) \
	texture_ ## NAME .Sample(sampler_ ## NAME , UV * uv_transform_ ## NAME .xy + uv_transform_ ## NAME  .zw)
	
static float2 neigbors_8[8] =
{
	float2(1, 0),
	float2(-1, 0),
	float2(0, 1),
	float2(0,-1),
	float2(-1,-1),
	float2(1,-1),
	float2(1, 1),
	float2(-1, 1)
};

float3 depthToPosition(float depth, float2 uv, float4x4 projectionInv)
{
	float4 position = mul(projectionInv, float4(uv, depth, 1.0f));
	return position.xyz / position.w;
}
float luma(float3 col)
{
	return 0.299 *  col.r + 0.587 * col.g + 0.114 * col.b;
}
float3 srgb(float3 v)
{
	return float3(pow(v.x, 0.45), pow(v.y, 0.45), pow(v.z, 0.45));
}
float4 srgb(float4 v)
{
	return float4(pow(v.x, 0.45), pow(v.y, 0.45), pow(v.z, 0.45), pow(v.w, 0.45));
}
float3 srgbInv(float3 v)
{
	return float3(pow(v.x, 2.2), pow(v.y, 2.2), pow(v.z, 2.2));
}
float4 srgbInv(float4 v)
{
	return float4(pow(v.x, 2.2), pow(v.y, 2.2), pow(v.z, 2.2), pow(v.w, 2.2));
}

//
// Tonemapping
//
float3 tonemapReinhard(float3 x)
{
	return x / (1.0 + luma(x));
}

// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
float3 tonemapUnreal(float3 x)
{
	return x / (x + 0.155) * 1.019;
}

float3 tonemapACES(float3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}


//
// lighting
//
float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float spec = 1.0 - roughness;
	return F0 + (max(float3(spec, spec, spec), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
float distributionGGX(float3 N, float3 H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.001);
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = 3.141592 * denom * denom;
	
	return num / denom;
}
float geometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	
	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return nom / denom;
}
float geometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

float radicalInverseVdC(uint bits)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
	bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
	bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
	return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
 }

float2 hammersley2d(int i, int N)
{
	return float2(float(i)/float(N), radicalInverseVdC(i));
}

float2 importanceSamplePhong(float2 xi, float a)
{
	float phi = 2.0f * 3.141592f * xi.x;
	float theta = acos(pow(1.0f - xi.y, 1.0f / (a + 1.0f)));
	return float2(phi, theta);
}

float3 importanceSampleGGX1(float4 s, float Roughness2, float3 N)
{
	float a = Roughness2;
	//float Phi = 2 * 3.1415926 * Xi.x;
	float CosTheta = sqrt((1 - s.z) / (1 + (a * a - 1) * s.z));
	float SinTheta = sqrt(1 - CosTheta * CosTheta);
	float3 H;
	H.x = SinTheta * s.x;// cos(Phi);
	H.y = SinTheta * s.y; // sin(Phi);
	H.z = CosTheta;
	float3 UpVector = abs(N.z) < 0.9999 ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);
	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 sphericalToCartesian(float2 s)
{
	return float3(sin(s.y) * cos(s.x), sin(s.y) * sin(s.x), cos(s.y));
}


// samples: cos(hamersley(i, N).x), sin(hamersley(i, N).x), hamersley(i, N).y, 0.0

#ifdef REFLECTION_GGX_LOW
	#define GGX_SAMPLES 10
	static const float4 samples[10] = {
		float4(1.000, 0.000, 0.000, 0.000),
		float4(0.809, 0.588, 0.500, 0.000),
		float4(0.309, 0.951, 0.250, 0.000),
		float4(-0.309, 0.951, 0.750, 0.000),
		float4(-0.809, 0.588, 0.125, 0.000),
		float4(-1.000, 0.000, 0.625, 0.000),
		float4(-0.809, -0.588, 0.375, 0.000),
		float4(-0.309, -0.951, 0.875, 0.000),
		float4(0.309, -0.951, 0.063, 0.000),
		float4(0.809, -0.588, 0.563, 0.000),
	};
#endif

#ifdef REFLECTION_GGX_MEDIUM
	#define GGX_SAMPLES 20
	static const float4 samples[20] = {
		float4(1.000, 0.000, 0.000, 0.000),
		float4(0.951, 0.309, 0.500, 0.000),
		float4(0.809, 0.588, 0.250, 0.000),
		float4(0.588, 0.809, 0.750, 0.000),
		float4(0.309, 0.951, 0.125, 0.000),
		float4(0.000, 1.000, 0.625, 0.000),
		float4(-0.309, 0.951, 0.375, 0.000),
		float4(-0.588, 0.809, 0.875, 0.000),
		float4(-0.809, 0.588, 0.063, 0.000),
		float4(-0.951, 0.309, 0.563, 0.000),
		float4(-1.000, 0.000, 0.313, 0.000),
		float4(-0.951, -0.309, 0.813, 0.000),
		float4(-0.809, -0.588, 0.188, 0.000),
		float4(-0.588, -0.809, 0.688, 0.000),
		float4(-0.309, -0.951, 0.438, 0.000),
		float4(-0.000, -1.000, 0.938, 0.000),
		float4(0.309, -0.951, 0.031, 0.000),
		float4(0.588, -0.809, 0.531, 0.000),
		float4(0.809, -0.588, 0.281, 0.000),
		float4(0.951, -0.309, 0.781, 0.000),
	};
#endif

#ifdef REFLECTION_GGX_HIGH
	#define GGX_SAMPLES 30
	static const float4 samples[30] = {
	float4(1.000, 0.000, 0.000, 0.000),
		float4(0.978, 0.208, 0.500, 0.000),
		float4(0.914, 0.407, 0.250, 0.000),
		float4(0.809, 0.588, 0.750, 0.000),
		float4(0.669, 0.743, 0.125, 0.000),
		float4(0.500, 0.866, 0.625, 0.000),
		float4(0.309, 0.951, 0.375, 0.000),
		float4(0.105, 0.995, 0.875, 0.000),
		float4(-0.105, 0.995, 0.063, 0.000),
		float4(-0.309, 0.951, 0.563, 0.000),
		float4(-0.500, 0.866, 0.313, 0.000),
		float4(-0.669, 0.743, 0.813, 0.000),
		float4(-0.809, 0.588, 0.188, 0.000),
		float4(-0.914, 0.407, 0.688, 0.000),
		float4(-0.978, 0.208, 0.438, 0.000),
		float4(-1.000, 0.000, 0.938, 0.000),
		float4(-0.978, -0.208, 0.031, 0.000),
		float4(-0.914, -0.407, 0.531, 0.000),
		float4(-0.809, -0.588, 0.281, 0.000),
		float4(-0.669, -0.743, 0.781, 0.000),
		float4(-0.500, -0.866, 0.156, 0.000),
		float4(-0.309, -0.951, 0.656, 0.000),
		float4(-0.105, -0.995, 0.406, 0.000),
		float4(0.105, -0.995, 0.906, 0.000),
		float4(0.309, -0.951, 0.094, 0.000),
		float4(0.500, -0.866, 0.594, 0.000),
		float4(0.669, -0.743, 0.344, 0.000),
		float4(0.809, -0.588, 0.844, 0.000),
		float4(0.914, -0.407, 0.219, 0.000),
		float4(0.978, -0.208, 0.719, 0.000),
	};
#endif

