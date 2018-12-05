
#include "lang/language.h"
#include "common.h"

UNIFORM_BUFFER_BEGIN(id)
	UNIFORM(uint, model_id)
UNIFORM_BUFFER_END

#ifdef ENG_SHADER_VERTEX

	///////////////////////
	// VERTEX SHADER
	///////////////////////

	#include "vertex.h"


#elif ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////
	
	MAIN_FRAG_UI(VS_OUTPUT)

		OUT_COLOR = model_id;
		
	MAIN_FRAG_END

#endif
