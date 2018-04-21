#version 330

layout(location = 0) in vec3 Position;

#ifdef ENG_INPUT_NORMAL
	layout(location = 1) in vec3 Normal;
#endif

#ifdef ENG_INPUT_TEXCOORD
	layout(location = 2) in vec2 TexCoord;
#endif

uniform mat4 MVP;

#ifdef ENG_INPUT_NORMAL
	uniform mat4 NM;
#endif

#ifdef ENG_INPUT_NORMAL
	smooth out vec3 N;
#endif

#ifdef ENG_INPUT_TEXCOORD
	smooth out vec2 UV;
#endif

void main()
{
	#ifdef ENG_INPUT_NORMAL
		N = (NM * vec4(Normal, 0)).xyz;
	#endif
	
	#ifdef ENG_INPUT_TEXCOORD
		UV = TexCoord;
	#endif

	gl_Position = MVP * vec4(Position, 1.0);
}
