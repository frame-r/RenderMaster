
#ifdef ENG_OPENGL
	#include "language_gl.h"
#elif ENG_DIRECTX11
	#include "language_dx11.h"
#else
	#error "Unknown language"
#endif
