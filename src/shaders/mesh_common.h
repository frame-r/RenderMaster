
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
		ATTRIBUTE(vec3, Normal, TEXCOORD1)
	#endif
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE(vec2, TexCoord, TEXCOORD2)
	#endif
	#ifdef ENG_INPUT_COLOR
		ATTRIBUTE(vec4, Color, TEXCOORD3)
	#endif
END_STRUCT
