
#include "language.h"

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

#ifdef ENG_SHADER_VERTEX

	///////////////////////
	// VERTEX SHADER
	///////////////////////

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

#elif ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////

	#ifdef ENG_INPUT_TEXCOORD
		TEXTURE2D_IN(texture0, 0)
	#endif


	MAIN_FRAG(VS_OUTPUT)
		vec4 ambient = vec4(0.1f, 0.1f, 0.1f, 0.0f);

		vec4 diffuse = main_color;

		#ifdef ENG_INPUT_NORMAL
			vec3 nN = normalize(GET_ATRRIBUTE(Normal).rgb);
			diffuse = diffuse * max(dot(nN, nL.rgb), 0);
		#endif

		#ifdef ENG_INPUT_TEXCOORD
			vec4 tex = TEXTURE(texture0, GET_ATRRIBUTE(TexCoord));
			tex = pow(tex, vec4(0.45f, 0.45f, 0.45f, 1.0f));
			diffuse = diffuse * tex;
		#endif

		#ifdef ENG_INPUT_COLOR
			diffuse = diffuse * GET_ATRRIBUTE(Color);
		#endif

		#ifdef ENG_ALPHA_TEST && ENG_INPUT_TEXCOORD
			if (tex.a < 0.5f)
				discard;
		#endif

		OUT_COLOR = diffuse + ambient;
		
		// make SRGB corection later in posteffect
		//OUT_COLOR = pow(OUT_COLOR, vec4(2.2f, 2.2f, 2.2f, 2.2f));
		
	MAIN_FRAG_END

#endif
