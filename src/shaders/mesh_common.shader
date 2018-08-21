// Constant buffer
UNIFORM_BUFFER_BEGIN(0)
	UNIFORM(vec4, main_color)
	UNIFORM(vec4, nL)
	UNIFORM(mat4, NM)
	UNIFORM(mat4, MVP)
UNIFORM_BUFFER_END


// Iterpolated Attributes
STRUCT(VS_OUTPUT)
	INIT_POSITION
	#ifdef ENG_INPUT_NORMAL
		ATTRIBUTE(0, vec3, Normal, TEXCOORD)
	#endif
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE(1, vec2, TexCoord, TEXCOORD)
	#endif
	#ifdef ENG_INPUT_COLOR
		ATTRIBUTE(2, vec4, Color, TEXCOORD)
	#endif
END_STRUCT
