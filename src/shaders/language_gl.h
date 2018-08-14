#version 330
// Defines provided by engine:

// ---Necessarily---
// ENG_SHADER_VERETX or ENG_SHADER_PIXEL
// ENG_OPENGL ENG_DIRECTX11

// ---Optional---
// ENG_INPUT_NORMAL
// ENG_INPUT_TEXCOORD
// ENG_INPUT_COLOR
// ENG_ALPHA_TEST


#define STRUCT(NAME)
#define END_STRUCT
#define ATTRIBUTE_VERETX_IN(NUM, TYPE, NAME, SEMANTIC) layout(location = NUM) in TYPE NAME;

#ifdef ENG_SHADER_VERTEX
#define ATTRIBUTE(NUM, TYPE, NAME, SEMANTIC) smooth out TYPE NAME;
#elif ENG_SHADER_PIXEL
#define ATTRIBUTE(NUM, TYPE, NAME, SEMANTIC) smooth in TYPE NAME;
#endif

#define CONSTANT_BUFFER_BEGIN(NAME)
#define CONSTANT_BUFFER_END
#define CONSTANT(TYPE, NAME) uniform TYPE NAME;

#define INIT_POSITION
#define POSITION_OUT gl_Position

#define TEXTURE2D_IN(NAME, NUM) uniform sampler2D NAME;
#define TEXTURE(NAME, UV) texture(NAME, UV);

#ifdef ENG_SHADER_PIXEL
out vec4 COLOR_OUT;
#endif

#define FRAGMENT_OUT
#define FRAGMENT_IN
#define VERTEX_OUT
#define VERTEX_IN
#define MAIN(VERTEX_IN_, VERTEX_OUT_) void main() {
#define MAIN_END }
#define MAIN_FRAG(FRAG_IN) void main() {
#define MAIN_FRAG_END }

