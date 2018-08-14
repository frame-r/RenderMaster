
// Defines provided by engine:

// ---Necessarily---
// ENG_SHADER_VERETX or ENG_SHADER_PIXEL
// ENG_OPENGL ENG_DIRECTX11

// ---Optional---
// ENG_INPUT_NORMAL
// ENG_INPUT_TEXCOORD
// ENG_INPUT_COLOR
// ENG_ALPHA_TEST

#define iint2 vec2
#define iint3 vec3
#define iint4 ivec4
#define uvec2 uint2
#define uvec3 uint3
#define uvec4 uint4
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat2 float2x2
#define mat3 float3x3
#define mat4 float4x4


#define STRUCT(NAME) struct NAME {
#define END_STRUCT };
#define ATTRIBUTE_VERETX_IN(NUM, TYPE, NAME, SEMANTIC) TYPE data_ ## NUM : SEMANTIC ## NUM;

#define ATTRIBUTE(NUM, TYPE, NAME, SEMANTIC) TYPE data_ ## NUM : SEMANTIC ## NUM;

#define CONSTANT_BUFFER_BEGIN(NAME) cbuffer NAME {
#define CONSTANT_BUFFER_END
#define CONSTANT(TYPE, NAME) TYPE NAME;

#define INIT_POSITION float4 pos_out : SV_POSITION;
#define POSITION_OUT pos_out.position

#define TEXTURE2D_IN(NAME, NUM) Texture2D s_texture_ ## NAME : register(t ## NUM);
#define TEXTURE(NAME, UV) s_texture_ ## NAME.Sample(s_sampler_ ## NAME, UV)

#define COLOR_OUT color

#define MAIN(VERTEX_IN, VERTEX_OUT) VERTEX_OUT void main(VERTEX_IN vs_input) { \
VERTEX_OUT input;

#define MAIN_END return input; }

#define MAIN_FRAG(FRAG_IN) float4 main(FRAG_IN input) : SV_TARGET \
{ \
float4 color;

#define MAIN_FRAG_END return color; \
}


