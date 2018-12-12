#include "../common/language.h"
#include "../common/common_post.h"

#ifdef ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////	
	
	TEXTURE2D_IN(0, TEX_COLOR)
	
	MAIN_FRAG(VS_OUTPUT)

		vec2 uv = GET_ATRRIBUTE(TexCoord);
		
		OUT_COLOR = TEXTURE(0, uv);
		
	MAIN_FRAG_END

#endif
