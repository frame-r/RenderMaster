#include "lang/language.h"
#include "common/common.h"

#ifdef ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////

	UNIFORM_BUFFER_BEGIN(id)
		UNIFORM(uint, model_id)
	UNIFORM_BUFFER_END
	
	MAIN_FRAG_UI(VS_OUTPUT)

		OUT_COLOR = model_id;
		
	MAIN_FRAG_END

#endif
