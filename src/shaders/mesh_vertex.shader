///////////////////////
// VERTEX SHADER
///////////////////////

#include <mesh_common.shader>


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
	POSITION_OUT = mul(MVP, GET_IN_ATTRIBUTE(PositionIn));

	#ifdef ENG_INPUT_NORMAL
		SET_OUT_ATTRIBUTE(Normal) = (mul(NM, vec4(GET_IN_ATTRIBUTE(NormalIn).xyz, 0.0f))).xyz;
	#endif

	#ifdef ENG_INPUT_TEXCOORD
		SET_OUT_ATTRIBUTE(TexCoord) = GET_IN_ATTRIBUTE(TexCoordIn);
	#endif

	#ifdef ENG_INPUT_COLOR
		SET_OUT_ATTRIBUTE(Color) = GET_IN_ATTRIBUTE(ColorIn);
	#endif	

MAIN_VERTEX_END


