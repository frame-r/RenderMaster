#include "common.h"
#include "vertex_post.h"

#ifdef ENG_SHADER_PIXEL

	Texture2D texture_normals : register(t0);
	Texture2D texture_shading : register(t1);
	Texture2D texture_albedo : register(t2);
	Texture2D texture_depth : register(t3);

	cbuffer light_parameters
	{
		float4 light_color;
		float4 light_direction;
	};

	cbuffer camera_parameters
	{
		float4 camera_position;
		float4x4 camera_view_projection_inv;
	};

	struct FS_OUT
	{
		float4 diffuse : SV_Target0;
		float4 specular : SV_Target1;
	};

	FS_OUT mainFS(VS_OUT fs_input, float4 screenPos : SV_Position)
	{
		FS_OUT out_color;
		
		float depth = texture_depth.Load(int3(screenPos.xy, 0)).r;
		if (depth == 1.0f)
			discard;

		float4 shading = texture_shading.Load(int3(screenPos.xy, 0));
		float roughness = shading.r;
		float metallic = shading.g;
		float4 albedo = texture_albedo.Load(int3(screenPos.xy, 0));
		float3 N = texture_normals.Load(int3(screenPos.xy, 0)).rgb * 2.0 - float3(1, 1, 1);

		float3 WorldPosition = getDepthToPosition(depth, fs_input.ndc, camera_view_projection_inv);
		float3 V = normalize(camera_position.xyz - WorldPosition);
		float NdotV = max(dot(N, V), 0.0);

		float3 F0 = float3(1, 1, 1) * 0.16 * Reflectance * Reflectance;
		F0 = lerp(F0, albedo, metallic);
		
		float3 L = light_direction.xyz;
		float NdotL = max(dot(N, L), 0.0);

		float roughness_analytic = roughness * roughness; // use ^2 to match environment specular
		
		float3 H = normalize(L + V);
		float VdotH = max(dot(V, H), 0.0);

		float D = DistributionGGX(N, H, roughness_analytic);
		float G = GeometrySmith(N, V, L, roughness_analytic);
		float3 F = fresnelSchlick(VdotH, F0);
		
		float3 specularBRDF = D * G * F /* NdotL*/ / (4 * NdotV /** NdotL*/ + 0.001);
		
		out_color.diffuse = float4(light_color.rgb * albedo.rgb * NdotL / 3.141592f, 0.0);
		out_color.specular = float4(light_color.rgb * specularBRDF, 0.0);

		return out_color;
	}

#endif
