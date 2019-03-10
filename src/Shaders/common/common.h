#ifndef H_COMMON
#define H_COMMON

	// Constant buffer
	UNIFORM_BUFFER_BEGIN(vertex_transformation_parameters)
		UNIFORM(mat4, MVP)
		UNIFORM(mat4, M)
		UNIFORM(mat4, NM)
	UNIFORM_BUFFER_END


	// Iterpolated Attributes
	STRUCT(VS_OUTPUT)
		INIT_POSITION
		ATTRIBUTE(vec3, WorldPosition, TEXCOORD1)
		#ifdef ENG_INPUT_NORMAL
			ATTRIBUTE(vec3, Normal, TEXCOORD2)
		#endif
		#ifdef ENG_INPUT_TEXCOORD
			ATTRIBUTE(vec2, TexCoord, TEXCOORD3)
		#endif
		#ifdef ENG_INPUT_COLOR
			ATTRIBUTE(vec4, Color, TEXCOORD4)
		#endif
	END_STRUCT


#ifdef ENG_SHADER_VERTEX

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


	// Default implementation of veretx shader
	MAIN_VERTEX(VS_INPUT, VS_OUTPUT)

		OUT_POSITION = mul(MVP, IN_ATTRIBUTE(PositionIn));
		
		vec4 WorldPosition4 = mul(M, IN_ATTRIBUTE(PositionIn));
		OUT_ATTRIBUTE(WorldPosition) = WorldPosition4.xyz;

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

#endif
#endif // H_COMMON
