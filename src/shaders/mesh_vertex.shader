///////////////////////
// VERTEX SHADER
///////////////////////

#include <mesh_common.shader>


// Input Attributes
ATTRIBUTE_VERETX_IN(0, vec3, PositionIn)
#ifdef ENG_INPUT_NORMAL
ATTRIBUTE_VERETX_IN(1, vec3, NormalIn)
#endif
#ifdef ENG_INPUT_TEXCOORD
ATTRIBUTE_VERETX_IN(2, vec2, TexCoordIn)
#endif
#ifdef ENG_INPUT_COLOR
ATTRIBUTE_VERETX_IN(3, vec4, ColorIn)
#endif

MAIN(VERTEX_OUT, VERTEX_IN)
	POSITION_OUT = MVP * vec4(PositionIn, 1.0f);

	#ifdef ENG_INPUT_NORMAL
		Normal = (NM * vec4(NormalIn, 0.0f)).xyz;
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		TexCoord = TexCoordIn;
	#endif

	#ifdef ENG_INPUT_COLOR
		Color = ColorIn;
	#endif	
MAIN_END
