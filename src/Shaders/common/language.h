#ifndef H_LANGUAGE
#define H_LANGUAGE

// Defines provided by engine:

// ---Necessarily---
// ENG_SHADER_VERETX or ENG_SHADER_PIXEL
// ENG_OPENGL ENG_DIRECTX11

// ---Optional---
// ENG_INPUT_NORMAL
// ENG_INPUT_TEXCOORD
// ENG_INPUT_COLOR

#ifdef ENG_OPENGL
	#include "language_gl.h"
#elif ENG_DIRECTX11
	#include "language_dx11.h"
#else
	#error "Unknown language"
#endif

#endif
