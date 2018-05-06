#version 330

// defines:
// ENG_INPUT_NORMAL
// ENG_INPUT_TEXCOORD
// ENG_INPUT_COLOR
// ENG_ALPHA_TEST

#ifdef ENG_INPUT_NORMAL
	uniform vec3 nL;
	smooth in vec3 NormalOut;
#endif

#ifdef ENG_INPUT_TEXCOORD
	uniform sampler2D texture0;
	smooth in vec2 TexCoordOut;
#endif

#ifdef ENG_INPUT_COLOR
	smooth in vec3 ColorOut;
#endif
	
uniform vec4 main_color;

out vec4 color_out;


void main()
{
	const vec3 ambient = vec3(0.1f, 0.1f, 0.1f);

	vec3 diffuse = main_color.rgb;

	#ifdef ENG_INPUT_NORMAL
		vec3 nN = normalize(NormalOut);
		diffuse = diffuse * vec3(max(dot(nN, nL), 0));
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		vec4 tex = texture(texture0, TexCoordOut);
		tex.rgb = pow(tex.rgb, vec3(2.2f));
		diffuse = diffuse * tex.rgb;
	#endif

	#ifdef ENG_INPUT_COLOR
		diffuse = diffuse * ColorOut;
	#endif

	#ifdef ENG_ALPHA_TEST && ENG_INPUT_TEXCOORD
		if (tex.a < 0.5f)
			discard;
	#endif

	color_out = vec4(diffuse + ambient, 1);
}
