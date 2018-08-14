///////////////////////
// PIXEL SHADER
///////////////////////

#include <mesh_common.shader>


TEXTURE2D_IN(texture0, 0)


MAIN_FRAG(VS_OUTPUT)
	vec3 ambient = vec3(0.1f, 0.1f, 0.1f);

	vec3 diffuse = main_color.rgb;

	#ifdef ENG_INPUT_NORMAL
		vec3 nN = normalize(Normal);
		diffuse = diffuse * vec3(max(dot(nN, nL), 0));
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		vec4 tex = TEXTURE(texture0, TexCoord);
		tex.rgb = pow(tex.rgb, vec3(2.2f));
		diffuse = diffuse * tex.rgb;
	#endif

	#ifdef ENG_INPUT_COLOR
		diffuse = diffuse * Color.rgb;
	#endif

	#ifdef ENG_ALPHA_TEST && ENG_INPUT_TEXCOORD
		if (tex.a < 0.5f)
			discard;
	#endif

	COLOR_OUT = vec4(diffuse + ambient, 1);
MAIN_FRAG_END
