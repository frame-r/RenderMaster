#version 420
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
#define ATTRIBUTE(TYPE, NAME, SEMANTIC) smooth out TYPE NAME;
#elif ENG_SHADER_PIXEL
#define ATTRIBUTE(TYPE, NAME, SEMANTIC) smooth in TYPE NAME;
#endif

//
// Uniform Blocks
// Note: use types multiplied 2 or 4 (not vec3!)
// Note: Don't use ifdef in uniform buffer block to match C++ side struct
#define UNIFORM_BUFFER_BEGIN(SLOT) layout (std140, binding = SLOT, row_major) uniform const_buffer_ ## SLOT \
{ 
#define UNIFORM_BUFFER_END };
#define UNIFORM(TYPE, NAME) uniform TYPE NAME;

#define INIT_POSITION
#define POSITION_OUT gl_Position

#define TEXTURE2D_IN(NAME, NUM) uniform sampler2D NAME;
#define TEXTURE(NAME, UV) texture(NAME, UV);

#ifdef ENG_SHADER_PIXEL
out vec4 COLOR_OUT;
#endif

// vertex
#define SET_OUT_ATTRIBUTE(NAME) NAME
#define GET_IN_ATTRIBUTE(NAME) NAME

// fragment 
#define GET_ARRIBUTE(NAME) NAME

#define FRAGMENT_OUT
#define FRAGMENT_IN
#define VERTEX_OUT
#define VERTEX_IN
#define MAIN_VERTEX(VERTEX_IN_, VERTEX_OUT_) void main() {
#define MAIN_VERTEX_END }
#define MAIN_FRAG(FRAG_IN) void main() {
#define MAIN_FRAG_END }

// math
#define mul(M, V) M * V


