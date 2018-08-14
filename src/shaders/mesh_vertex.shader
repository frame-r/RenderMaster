///////////////////////
// VERTEX SHADER
///////////////////////

#include <mesh_common.shader>


// Input Attributes
STRUCT(VS_INPUT)
		ATTRIBUTE_VERETX_IN(0, vec3, PositionIn, POSITION)
	#ifdef ENG_INPUT_NORMAL
		ATTRIBUTE_VERETX_IN(1, vec3, NormalIn, TEXOORD)
	#endif
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE_VERETX_IN(2, vec2, TexCoordIn, TEXOORD)
	#endif
	#ifdef ENG_INPUT_COLOR
		ATTRIBUTE_VERETX_IN(3, vec4, ColorIn, TEXOORD)
	#endif
END_STRUCT


MAIN(VS_INPUT, VS_OUTPUT)
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
