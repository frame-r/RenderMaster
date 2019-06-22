#include "common.h"
#include "vertex_post.h"

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

		float3x3 ReflRotMat = float3x3(float3(1,0,0), float3(0,0,1), float3(0,1,0));

		//
		// Environment specular
		//
#ifdef REFLECTION_MIPMAP
		float3 specularEnv = float3(0, 0, 0);
		{ // fast reflection (mipmap)
			float3 L = normalize(reflect(-V, N));

			const float specularMip = sqrt(roughness) * (environment_resolution.z - 1);

			if (depth != 1)
				specularEnv = texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, L), specularMip).rgb;

			float3 H = normalize(L + V);
			float NdotL = max(dot(N, L), 0.0);
			float NdotV = max(dot(N, V), 0.0);
			float VdotH = max(dot(V, H), 0.0);
			float3 D = DistributionGGX(N, H, roughness);
			float3 G = GeometrySmith(N, V, L, roughness);
			float3 F = fresnelSchlick(VdotH, F0);

			specularEnv = specularEnv * F * G  / (NdotV + 0.001);
		}
#else // REFLECTION_GGX
		float3 specularEnv = float3(0, 0, 0);
		{ // Importance sampling

			roughness *= roughness;

			const float gloss = 1.0f - roughness;
			float a = max(roughness * roughness, 5e-4f );
			float a2 = a*a;
			float k = a * 0.5f;
			float3 basisX = abs(N.z < 0.9999) ? normalize(cross(float3(0, 0, 1), N)) : float3(1, 0, 0);
			float3 basisY = cross(N, basisX);
			float3 basisZ = N;

			float A = 0.5f * log2((environment_resolution.x * environment_resolution.y) / float(GGXsamples)) + 1.5f * gloss * gloss;

			//[unroll]
			for(int i = 0; i < GGXsamples; i++)
			{
				float3 dir = ImportanceSampleGGX( hammersleySamples[i], a2 -5e-4f *5e-4f);
				float3 H = basisX * dir.x + basisY * dir.y + basisZ * dir.z;
				float3 L = H * (2.0f * dot(V,H)) - V;

				float NdotH = saturate( dot(N, H) );
				float NdotL = saturate( dot(N, L) );
				float VdotH = saturate( dot(V, H) );
		
				float d = ( NdotH * a2 - NdotH ) * NdotH + 1.0f;
				float pdf = (NdotH * a2) / (4.0f * 3.141593f * d* d * VdotH);

				float lod = A - 0.5f * log2(pdf);
				float3 sampleCol = float3(0, 0, 0);
				if (depth != 1)
					sampleCol = texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, L), lod ).xyz;

				float3 G = GeometrySmith(N, V, L, roughness);
				float3 F = fresnelSchlick(VdotH, F0);

				specularEnv += sampleCol * F * G / (NdotV + 0.001);
			}

			specularEnv /= float(GGXsamples);
		}
#endif

		specularEnv /= 1.5f; // i don't know what is it. but this coeff gives more natural intensity
		// TODO: clarify maybe it is some sort of 3.1415 or 4.0

		//
		// Environment diffuse
		//
		float3 diffuseEnv = float3(0,0,0);
		{
			float3 NotCollinearToN = normalize(lerp(float3(-N.y,N.x,0), float3(1,0,0), abs(N.z)));
			float3 basisX = normalize(cross(NotCollinearToN, N));
			float3 basisY = cross(N, basisX);

			if (depth != 1)
			{
				const float ambientlMip = environment_resolution.z - 2;

				const float ambientSampels = 5;

				const float d = 0.1;
				float3 sample2 = normalize( d * basisX + d * basisY + N);
				float3 sample3 = normalize(-d * basisX + d * basisY + N);
				float3 sample4 = normalize(-d * basisX - d * basisY + N);
				float3 sample5 = normalize( d * basisX - d * basisY + N);

				diffuseEnv +=					texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, N), ambientlMip).rgb;
				diffuseEnv += dot(N, sample2) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample2), ambientlMip).rgb;
				diffuseEnv += dot(N, sample3) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample3), ambientlMip).rgb;
				diffuseEnv += dot(N, sample4) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample4), ambientlMip).rgb;
				diffuseEnv += dot(N, sample5) * texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, sample5), ambientlMip).rgb;
				
				diffuseEnv *= albedo / (ambientSampels * 3.1415f);
			}
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
		float3 bkg = texture_environment.SampleLevel(sampler_environment, mul(ReflRotMat, -V), 0).rgb;

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
