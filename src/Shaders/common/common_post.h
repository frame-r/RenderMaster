#ifndef H_COMMON
#define H_COMMON

	// Constant buffer
	UNIFORM_BUFFER_BEGIN(viewport_parameters)
		UNIFORM(uint, width)
		UNIFORM(uint, height)
		UNIFORM(float, invWidth)
		UNIFORM(float, invHeight)
	UNIFORM_BUFFER_END

	// Iterpolated Attributes
	STRUCT(VS_OUTPUT)
		INIT_POSITION
		ATTRIBUTE(vec2, TexCoord, TEXCOORD2)
	END_STRUCT


#ifdef ENG_SHADER_VERTEX

	// Input Attributes
	STRUCT(VS_INPUT)
		ATTRIBUTE_VERETX_IN(0, vec4, PositionIn, POSITION0)
	END_STRUCT

	// Default implementation of veretx shader
	MAIN_VERTEX(VS_INPUT, VS_OUTPUT)
		
		vec2 pos = IN_ATTRIBUTE(PositionIn).xy;
		
		#ifdef ENG_DIRECTX11
			pos.y = -pos.y;
		#endif
		
		OUT_POSITION = vec4(pos, 0.0f, 1.0f);
		
		vec4 tex = IN_ATTRIBUTE(PositionIn);
		vec2 tex2d = tex.xy * vec2(0.5f, 0.5f) + vec2(0.5f, 0.5f);
		OUT_ATTRIBUTE(TexCoord) = tex2d;

	MAIN_VERTEX_END

#endif
#endif // H_COMMON
