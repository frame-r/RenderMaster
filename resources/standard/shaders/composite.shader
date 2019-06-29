#include "common.h"
#include "vertex_post.h"

//#define WHITE_BAKGROUND


#ifdef ENG_SHADER_PIXEL

	Texture2D texture_albedo : register(t0);
	Texture2D texture_normals : register(t1);
	Texture2D texture_shading : register(t2);
	Texture2D texture_diffuse_light : register(t3);
	Texture2D texture_specular_light : register(t4);
	Texture2D texture_depth : register(t5);
	TextureCube texture_environment : register(t6);
	SamplerState sampler_environment : register(s6);	

	cbuffer parameters
	{
		float4 environment_resolution; // w, h, mipmaps, 0
		float4 environment_intensity; // diffuse, specular, 0, 0
	};
	static float3x3 ReflRotMat = float3x3(float3(1, 0, 0), float3(0, 0, 1), float3(0, 1, 0));

	cbuffer camera_parameters
	{
		float4 camera_position;
		float4x4 camera_view_projection_inv;
		float4x4 camera_view_inv;
	};

	float4 mainFS(VS_OUT fs_input, float4 screenPos : SV_Position) : SV_Target0
	{
		float depth = texture_depth.Load(int3(screenPos.xy, 0)).r;
		float3 albedo = texture_albedo.Load(int3(screenPos.xy, 0)).rgb;
		float3 N = texture_normals.Load(int3(screenPos.xy, 0)).rgb * 2.0 - float3(1, 1, 1);
		float4 shading = texture_shading.Load(int3(screenPos.xy, 0));
		float3 diffuseBRDF = texture_diffuse_light.Load(int3(screenPos.xy, 0)).rgb;
		float3 specularBRDF = texture_specular_light.Load(int3(screenPos.xy, 0)).rgb;

		float roughness = shading.r;
		float metallic = shading.g;
		float3 WorldPosition = getDepthToPosition(depth, fs_input.ndc, camera_view_projection_inv);
		float3 V = normalize(camera_position.xyz - WorldPosition);
		float NdotV = max(dot(N, V), 0.0);

		// Reflectance at normal incidence (for metals use albedo color)
		float3 F0 = float3(1, 1, 1) * 0.16 * Reflectance * Reflectance;
		F0 = lerp(F0, albedo, metallic);

		float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
		float3 kD = lerp(float3(1, 1, 1) - F, float3(0, 0, 0), metallic);
		
		//
		// Environment specular
		//
		
		float3 specularEnv = float3(0, 0, 0);
		
	#ifdef REFLECTION_MIPMAP

		float3 L = normalize(reflect(-V, N));
		float specularMip = sqrt(roughness) * (environment_resolution.z - 1);

		if (depth != 1)
			#ifdef WHITE_BAKGROUND
				specularEnv = float3(0.5, 0.5, 0.5);
			#else
				specularEnv = texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, L), specularMip).rgb;
			#endif
		

	#else // REFLECTION_GGX

		roughness += 0.02;
		float roughness2 = roughness * roughness;
		float gloss = 1.0f - roughness;
		float A = 0.5f * log2((environment_resolution.x * environment_resolution.y) / float(GGX_SAMPLES)) + 1.5f * gloss * gloss;
		float weights = 0;

		for (uint i = 0; i < GGX_SAMPLES; i++)
		{
			//float2 Xi = Hammersley2d(i, NumSamples);
			float4 s = samples[i];

			float3 H = ImportanceSampleGGX1(s, roughness2, N);
			
			float3 L = 2 * dot(V, H) * H - V;
			float NoV = saturate(dot(N, V));
			//float NoL = saturate(dot(N, L));
			float NoH = saturate(dot(N, H));
			//float VoH = saturate(dot(V, H));

			//if (NoL > 0)
			{
				float pdf = DistributionGGX(N, H, roughness) * NoH / (4 * NoV);
				float lod = A - 0.5f * log2(pdf);
				lod += 0.5 * pow(roughness, 2);

#ifdef WHITE_BAKGROUND
				float3 SampleColor = float3(0.5, 0.5, 0.5);
#else
				float3 SampleColor = texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, L), lod).rgb;
#endif
				//float G = GeometrySmith(N, V, L, Roughness);
				//float Fc = pow(1 - VoH, 5);
				//float3 F = (1 - Fc) * SpecularColor + Fc;
				// Incident light = SampleColor * NoL
				// Microfacet specular = D*G*F / (4*NoL*NoV)
				// pdf = D * NoH / (4 * VoH)
				//float w = 1;// F* G* VoH / (NoH * NoV);
				//weights += w;

				specularEnv += SampleColor; // * w;
			}
		}
		//specularEnv /= weights;
		specularEnv /= float(GGX_SAMPLES);
		
	#endif

		specularEnv *= F;

		//
		// Environment diffuse
		//
		float3 diffuseEnv = float3(0,0,0);

		if (depth != 1)
		{
			float3 NotCollinearToN = normalize(lerp(float3(-N.y,N.x,0), float3(1,0,0), abs(N.z)));
			float3 basisX = normalize(cross(NotCollinearToN, N));
			float3 basisY = cross(N, basisX);
			
			const float ambientlMip = environment_resolution.z - 2;
			const float ambientSampels = 5;
			const float d = 0.1;
			
			float3 sample2 = normalize( d * basisX + d * basisY + N);
			float3 sample3 = normalize(-d * basisX + d * basisY + N);
			float3 sample4 = normalize(-d * basisX - d * basisY + N);
			float3 sample5 = normalize( d * basisX - d * basisY + N);

			#ifdef WHITE_BAKGROUND
				diffuseEnv +=					float3(0.5,0.5,0.5);
				diffuseEnv += dot(N, sample2) * float3(0.5,0.5,0.5);
				diffuseEnv += dot(N, sample3) * float3(0.5,0.5,0.5);
				diffuseEnv += dot(N, sample4) * float3(0.5,0.5,0.5);
				diffuseEnv += dot(N, sample5) * float3(0.5,0.5,0.5);
			#else
				diffuseEnv +=					texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, N), ambientlMip).rgb;
				diffuseEnv += dot(N, sample2) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample2), ambientlMip).rgb;
				diffuseEnv += dot(N, sample3) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample3), ambientlMip).rgb;
				diffuseEnv += dot(N, sample4) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample4), ambientlMip).rgb;
				diffuseEnv += dot(N, sample5) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample5), ambientlMip).rgb;
			#endif
			
			diffuseEnv *= albedo / (ambientSampels);
		}


		//
		// Environment + lights
		//
		float3 color = float3(0, 0, 0);
		color += kD * environment_intensity.x * diffuseEnv;
		color += environment_intensity.y * specularEnv;
		color += kD * diffuseBRDF;
		color += specularBRDF;

		float3 R = normalize(reflect(-V, N));
		
		#ifdef WHITE_BAKGROUND
			float3 bkg = float3(0.5, 0.5, 0.5);
		#else
			float3 bkg = texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, -V), 0).rgb;
		#endif
		
		color = lerp(color, bkg, float(depth == 1.0f));

		color *= 1.0f; // exposure

		color = Tonemap_ACES(color);
		//color = Tonemap_Reinhard(color);
		color = srgb(color);
		//color = Tonemap_Unreal(color); // use without gamma correction!

		#ifdef VIEW_MODE_NORMAL
			return float4(N.x * 0.5 + 0.5, N.y * 0.5 + 0.5, N.z * 0.5 + 0.5, 1.0);
		#elif VIEW_MODE_ALBEDO
			return float4(albedo, 1.0);
		#elif VIEW_MODE_DIFFUSE_LIGHT
			return float4(kD * diffuseBRDF, 1.0);
		#elif VIEW_MODE_SPECULAR_LIGHT
			return float4(specularBRDF, 1.0);
		#else
			return float4(color, 1.0);
		#endif
	}

#endif
