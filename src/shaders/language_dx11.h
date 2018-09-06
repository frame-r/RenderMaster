
// Defines provided by engine:

// ---Necessarily---
// ENG_SHADER_VERETX or ENG_SHADER_PIXEL
// ENG_OPENGL ENG_DIRECTX11

// ---Optional---
// ENG_INPUT_NORMAL
// ENG_INPUT_TEXCOORD
// ENG_INPUT_COLOR
// ENG_ALPHA_TEST

#pragma pack_matrix( row_major )

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
#define ATTRIBUTE_VERETX_IN(NUM, TYPE, NAME, SEMANTIC) TYPE NAME : SEMANTIC;

#define ATTRIBUTE(TYPE, NAME, SEMANTIC) TYPE NAME : SEMANTIC;

//
// Constant Buffer
#define UNIFORM_BUFFER_BEGIN(SLOT) cbuffer const_buffer_##SLOT : register( b##SLOT ) {
#define UNIFORM_BUFFER_END };
#define UNIFORM(TYPE, NAME) TYPE NAME;

#define INIT_POSITION float4 position : SV_POSITION;
#define OUT_POSITION vs_output.position

#define TEXTURE2D_IN(NAME, NUM) Texture2D NAME : register(t ## NUM); \
SamplerState g_samLinear : register( s0 );

#define TEXTURE(NAME, UV) NAME.Sample(g_samLinear, UV)

#define OUT_COLOR color

// vertex in/out
#define OUT_ATTRIBUTE(NAME) vs_output.NAME
#define IN_ATTRIBUTE(NAME) vs_input.NAME

// fragment in
#define GET_ARRIBUTE(NAME) fs_input.NAME


#define MAIN_VERTEX(VERTEX_IN, VERTEX_OUT) VERTEX_OUT mainVS(VERTEX_IN vs_input) { \
VERTEX_OUT vs_output;

#define MAIN_VERTEX_END return vs_output; \
} \
  

#define MAIN_FRAG(FRAG_IN) float4 mainFS(FRAG_IN fs_input) : SV_TARGET \
{ \
float4 color;

#define MAIN_FRAG_END return color; \
} \
  

// math
#define mul(M, V) mul(M, V)

