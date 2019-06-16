
static const float Reflectance = 0.4;

float3 getDepthToPosition(float depth, float2 uv, float4x4 iprojection)
{
	float4 position = mul(iprojection, float4(uv, depth, 1.0f));
	return position.xyz / position.w;
}
float luma(float3 col)
{
	return 0.299*col.r + 0.587*col.g + 0.114*col.b;
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
float3 Tonemap_Reinhard(float3 x)
{
	// Reinhard et al. 2002, "Photographic Tone Reproduction for Digital Images", Eq. 3
	return x / (1.0 + luma(x));
}

float3 Tonemap_Unreal(float3 x)
{
	// Unreal, Documentation: "Color Grading"
	// Adapted to be close to Tonemap_ACES, with similar range
	// Gamma 2.2 correction is baked in, don't use with sRGB conversion!
	return x / (x + 0.155) * 1.019;
}
float3 Tonemap_ACES(float3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}


float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float spec = 1.0 - roughness;
    return F0 + (max(float3(spec,spec,spec), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
float DistributionGGX(float3 N, float3 H, float roughness)
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
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float radicalInverse_VdC(uint bits)
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
	return float2(float(i)/float(N), radicalInverse_VdC(i));
}
float2 importance_sample_phong(float2 xi, float a)
{
	float phi = 2.0f * 3.141592f * xi.x;
	float theta = acos(pow(1.0f - xi.y, 1.0f / (a + 1.0f)));
	return float2(phi, theta);
}
float3 toC(float2 s)
{
	return float3(sin(s.y) * cos(s.x), sin(s.y) * sin(s.x), cos(s.y));
}
float3 ImportanceSampleGGX( float4 r, float a2 )
{
	float cosTheta = sqrt((1.0f - r.x) / ((a2 - 1.0f) * r.x + 1.0f));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
	float cosPhi = r.z;
	float sinPhi = r.w;
	return float3(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
}

#if 0
static const int GGXsamples = 17;
static const float4 hammersleySamples [17] =
{
	float4(0.00, 0.00, 1.00, 0.00),
	//float4(0.02941, 0.50001, -1.00, -0.00005),
	float4(0.05882, 0.25, -0.00002, 1.00),
	//float4(0.08824, 0.75001, 0.00007, -1.00),
	float4(0.11765, 0.125, 0.7071, 0.70712),
	//float4(0.14706, 0.62501, -0.70706, -0.70715),
	float4(0.17647, 0.37501, -0.70713, 0.70708),
	//float4(0.20588, 0.87501, 0.70717, -0.70705),
	float4(0.23529, 0.0625, 0.92388, 0.38269),
	//float4(0.26471, 0.56251, -0.92386, -0.38273),
	float4(0.29412, 0.3125, -0.38271, 0.92387),
	//float4(0.32353, 0.81251, 0.38276, -0.92385),
	float4(0.35294, 0.1875, 0.38267, 0.92389),
	//float4(0.38235, 0.68751, -0.38262, -0.9239),
	float4(0.41176, 0.43751, -0.9239, 0.38264),
	//float4(0.44118, 0.93751, 0.92391, -0.3826),
	float4(0.47059, 0.03125, 0.98078, 0.19509),
	//float4(0.50, 0.53126, -0.98078, -0.19514),
	float4(0.52941, 0.28125, -0.19512, 0.98078),
	//float4(0.55882, 0.78126, 0.19516, -0.98077),
	float4(0.58824, 0.15625, 0.55556, 0.83148),
	//float4(0.61765, 0.65626, -0.55552, -0.8315),
	float4(0.64706, 0.40626, -0.83149, 0.55554),
	//float4(0.67647, 0.90626, 0.83152, -0.5555),
	float4(0.70588, 0.09375, 0.83146, 0.55558),
	//float4(0.73529, 0.59376, -0.83144, -0.55562),
	float4(0.76471, 0.34376, -0.5556, 0.83145),
	//float4(0.79412, 0.84376, 0.55564, -0.83142),
	float4(0.82353, 0.21875, 0.19507, 0.98079),
	//float4(0.85294, 0.71876, -0.19502, -0.9808),
	float4(0.88235, 0.46876, -0.98079, 0.19505),
	//float4(0.91176, 0.96876, 0.9808, -0.195),
	float4(0.94118, 0.01563, 0.99518, 0.09802),
	//float4(0.97059, 0.51563, -0.99518, -0.09807),
};
#else
static const int GGXsamples = 34;
static const float4 hammersleySamples [34] =
{
	float4(0.00, 0.00, 1.00, 0.00),
	float4(0.02941, 0.50001, -1.00, -0.00005),
	float4(0.05882, 0.25, -0.00002, 1.00),
	float4(0.08824, 0.75001, 0.00007, -1.00),
	float4(0.11765, 0.125, 0.7071, 0.70712),
	float4(0.14706, 0.62501, -0.70706, -0.70715),
	float4(0.17647, 0.37501, -0.70713, 0.70708),
	float4(0.20588, 0.87501, 0.70717, -0.70705),
	float4(0.23529, 0.0625, 0.92388, 0.38269),
	float4(0.26471, 0.56251, -0.92386, -0.38273),
	float4(0.29412, 0.3125, -0.38271, 0.92387),
	float4(0.32353, 0.81251, 0.38276, -0.92385),
	float4(0.35294, 0.1875, 0.38267, 0.92389),
	float4(0.38235, 0.68751, -0.38262, -0.9239),
	float4(0.41176, 0.43751, -0.9239, 0.38264),
	float4(0.44118, 0.93751, 0.92391, -0.3826),
	float4(0.47059, 0.03125, 0.98078, 0.19509),
	float4(0.50, 0.53126, -0.98078, -0.19514),
	float4(0.52941, 0.28125, -0.19512, 0.98078),
	float4(0.55882, 0.78126, 0.19516, -0.98077),
	float4(0.58824, 0.15625, 0.55556, 0.83148),
	float4(0.61765, 0.65626, -0.55552, -0.8315),
	float4(0.64706, 0.40626, -0.83149, 0.55554),
	float4(0.67647, 0.90626, 0.83152, -0.5555),
	float4(0.70588, 0.09375, 0.83146, 0.55558),
	float4(0.73529, 0.59376, -0.83144, -0.55562),
	float4(0.76471, 0.34376, -0.5556, 0.83145),
	float4(0.79412, 0.84376, 0.55564, -0.83142),
	float4(0.82353, 0.21875, 0.19507, 0.98079),
	float4(0.85294, 0.71876, -0.19502, -0.9808),
	float4(0.88235, 0.46876, -0.98079, 0.19505),
	float4(0.91176, 0.96876, 0.9808, -0.195),
	float4(0.94118, 0.01563, 0.99518, 0.09802),
	float4(0.97059, 0.51563, -0.99518, -0.09807),
};
#endif
