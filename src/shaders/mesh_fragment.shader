#version 330

#ifdef ENG_INPUT_NORMAL
	smooth in vec3 N;
#endif

#ifdef ENG_INPUT_TEXCOORD
	smooth in vec2 UV;
#endif

#ifdef ENG_INPUT_NORMAL
	uniform vec3 nL;
#endif

uniform vec4 main_color;

#ifdef ENG_INPUT_TEXCOORD
	uniform sampler2D texture0;
#endif

out vec4 color_out;


void main()
{
	#ifdef ENG_INPUT_NORMAL
		vec3 nN = normalize(N);
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		vec4 tex = texture(texture0, UV);
		tex.rgb = pow(tex.rgb, vec3(2.2f, 2.2f, 2.2f));
	#endif

	#ifdef ENG_ALPHA_TEST && ENG_INPUT_TEXCOORD
		if (tex.a <= 0.5)
			discard;
	#endif

	#ifdef ENG_INPUT_NORMAL && ENG_INPUT_TEXCOORD
		color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * tex * main_color;
	#elif ENG_INPUT_NORMAL && !ENG_INPUT_TEXCOORD
		color_out = vec4(vec3(max(dot(nN, nL), 0)), 1) * main_color;
	#elif ENG_INPUT_TEXCOORD
		color_out = tex * main_color;
	#else
		color_out = main_color;
	#endif

	color_out.rgb = pow(color_out.rgb, vec3(1.0f / 2.2f));

}
