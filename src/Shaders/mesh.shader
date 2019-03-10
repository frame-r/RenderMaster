#include "common/language.h"
#include "common/common.h"


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.141592 * denom * denom;
	
    return num / denom;
}

#ifdef ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////

	#ifdef ENG_INPUT_TEXCOORD
		TEXTURE2D_IN(0, TEX_ALBEDO)
	#endif

	UNIFORM_BUFFER_BEGIN(material_parameters)
		UNIFORM(vec4, main_color)
		UNIFORM(vec4, shading) // r - metallic, g - roughness
		UNIFORM(vec4, camera_position)
		UNIFORM(vec4, light_position)
		UNIFORM(vec4, nL_world)
	UNIFORM_BUFFER_END

	MAIN_FRAG(VS_OUTPUT)

		const vec4 ambient = vec4(0.05f, 0.05f, 0.05f, 0.0f);
		float metallic = shading.r;
		float roughness = shading.g;		

		vec4 diffuse = main_color;
		vec4 specular = vec4(0, 0, 0, 0);

		#ifdef ENG_INPUT_NORMAL
			vec3 N = normalize(GET_ATRRIBUTE(Normal).rgb);
			
			// diffuse
			diffuse = max(dot(N, nL_world.xyz), 0) * main_color;
			
			// specular
			vec3 L = nL_world.xyz;
			vec3 V = normalize(camera_position.xyz - GET_ATRRIBUTE(WorldPosition));
			vec3 H = normalize( L + V );
			
			specular = vec4(1, 1, 1, 0) * DistributionGGX(N, H, roughness) * max(dot(N, nL_world.xyz), 0);
			
		#endif

		#ifdef ENG_INPUT_TEXCOORD
			vec4 tex = TEXTURE(0, GET_ATRRIBUTE(TexCoord));
			tex = pow(tex, vec4(0.45f, 0.45f, 0.45f, 1.0f));
			diffuse = diffuse * tex;
		#endif

		OUT_COLOR = ambient + diffuse + specular;
		
		// debug normal
		//#ifdef ENG_INPUT_NORMAL
		//	OUT_COLOR.rgb = N * 0.5 + vec3(0.5, 0.5, 0.5);
		//#endif
		
	MAIN_FRAG_END

#endif
