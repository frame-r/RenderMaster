#include "common/language.h"
#include "common/common_post.h"

#ifdef ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////	
	
	vec3 pow3(vec3 v, float r)
	{
		return vec3(pow(v.x, r), pow(v.y, r), pow(v.z, r));
	}
	
	TEXTURE2D_IN(0, TEX_COLOR)
	
	MAIN_FRAG(VS_OUTPUT)

		vec2 uv = GET_ATRRIBUTE(TexCoord);
		vec3 retColor = TEXTURE(0, uv).rgb;
		
		retColor = retColor / (1.0f + retColor);
		
		//retColor = pow3(retColor, 0.45);
		
		OUT_COLOR = vec4(retColor, 1.0f);
		
	MAIN_FRAG_END

#endif
