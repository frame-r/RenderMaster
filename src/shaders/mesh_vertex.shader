///////////////////////
// VERTEX SHADER
///////////////////////

#include <mesh_common.h>


// Input Attributes
STRUCT(VS_INPUT)
		ATTRIBUTE_VERETX_IN(0, vec4, PositionIn, POSITION0)
	#ifdef ENG_INPUT_NORMAL
		ATTRIBUTE_VERETX_IN(1, vec4, NormalIn, TEXCOORD1)
	#endif
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE_VERETX_IN(2, vec2, TexCoordIn, TEXCOORD2)
	#endif
	#ifdef ENG_INPUT_COLOR
		ATTRIBUTE_VERETX_IN(3, vec4, ColorIn, TEXCOORD3)
	#endif
END_STRUCT


MAIN_VERTEX(VS_INPUT, VS_OUTPUT)
	
	OUT_POSITION = mul(MVP, IN_ATTRIBUTE(PositionIn));

	#ifdef ENG_INPUT_NORMAL
		OUT_ATTRIBUTE(Normal) = (mul(NM, vec4(IN_ATTRIBUTE(NormalIn).xyz, 0.0f))).xyz;
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		OUT_ATTRIBUTE(TexCoord) = IN_ATTRIBUTE(TexCoordIn);
	#endif

	#ifdef ENG_INPUT_COLOR
		OUT_ATTRIBUTE(Color) = IN_ATTRIBUTE(ColorIn);
	#endif	

MAIN_VERTEX_END


