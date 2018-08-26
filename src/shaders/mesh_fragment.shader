///////////////////////
// PIXEL SHADER
///////////////////////

#include <mesh_common.shader>


TEXTURE2D_IN(texture0, 0)


MAIN_FRAG(VS_OUTPUT)
	vec4 ambient = vec4(0.1f, 0.1f, 0.1f, 0.0f);

	vec4 diffuse = main_color;

	#ifdef ENG_INPUT_NORMAL
		vec3 nN = normalize(GET_ARRIBUTE(Normal).rgb);
		diffuse = diffuse * max(dot(nN, nL.rgb), 0);
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		vec4 tex = TEXTURE(texture0, GET_ARRIBUTE(TexCoord));
		tex = pow(tex.rgb, vec4(2.2f, 2.2f, 2.2f, 2.2f));
		diffuse = diffuse * tex;
	#endif

	#ifdef ENG_INPUT_COLOR
		diffuse = diffuse * GET_ARRIBUTE(Color);
	#endif

	#ifdef ENG_ALPHA_TEST && ENG_INPUT_TEXCOORD
		if (tex.a < 0.5f)
			discard;
	#endif

	COLOR_OUT = diffuse + ambient;
	
MAIN_FRAG_END


