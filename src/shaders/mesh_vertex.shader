#version 330

#ifdef ENG_INPUT_2D
	layout(location = 0) in vec2 Position;
#else
	layout(location = 0) in vec3 Position;
#endif

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
	
	#ifdef ENG_INPUT_2D
		gl_Position = MVP * vec4(Position.x, Position.y, 0.0, 1.0);
	#else
		gl_Position = MVP * vec4(Position, 1.0);
	#endif
}
