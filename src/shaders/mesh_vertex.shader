#version 330

// defines:
// ENG_INPUT_NORMAL
// ENG_INPUT_TEXCOORD
// ENG_INPUT_COLOR
// ENG_ALPHA_TEST

layout(location = 0) in vec3 PositionIn;

uniform mat4 MVP;

#ifdef ENG_INPUT_NORMAL
	uniform mat4 NM;
	layout(location = 1) in vec3 NormalIn;
	smooth out vec3 NormalOut;
#endif

#ifdef ENG_INPUT_TEXCOORD
	layout(location = 2) in vec2 TexCoordIn;
	smooth out vec2 TexCoordOut;
#endif

#ifdef ENG_INPUT_COLOR
	layout(location = 3) in vec3 ColorIn;
	smooth out vec3 ColorOut;
#endif


void main()
{
	gl_Position = MVP * vec4(PositionIn, 1.0f);

	#ifdef ENG_INPUT_NORMAL
		NormalOut = (NM * vec4(NormalIn, 0.0f)).xyz;
	#endif
	
	#ifdef ENG_INPUT_TEXCOORD
		TexCoordOut = TexCoordIn;
	#endif

	#ifdef ENG_INPUT_COLOR
		ColorOut = ColorIn;
	#endif	
}
