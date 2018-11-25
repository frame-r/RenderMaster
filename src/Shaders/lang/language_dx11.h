#ifndef H_LANGUAGE_DX11
#define H_LANGUAGE_DX11

//#pragma pack_matrix(row_major)
// We don't use this option beacuse we already specify flag
// for compilation "D3DCOMPILE_PACK_MATRIX_ROW_MAJOR"


// Math
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#define uvec2 uint2
#define uvec3 uint3
#define uvec4 uint4
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat2 float2x2
#define mat3 float3x3
#define mat4 float4x4

#define mul(M, V) mul(M, V)


#define STRUCT(NAME) struct NAME {
#define END_STRUCT };

#define ATTRIBUTE_VERETX_IN(NUM, TYPE, NAME, SEMANTIC) TYPE NAME : SEMANTIC;
#define ATTRIBUTE(TYPE, NAME, SEMANTIC) TYPE NAME : SEMANTIC;

// Uniform blocks (constant buffers)
#define UNIFORM_BUFFER_BEGIN(SLOT) cbuffer const_buffer_##SLOT : register( b##SLOT ) {
#define UNIFORM_BUFFER_END };
#define UNIFORM(TYPE, NAME) TYPE NAME;

// Textures
#define TEXTURE2D_IN(NAME, NUM) Texture2D NAME : register(t ## NUM); \
SamplerState g_samLinear : register( s0 );
#define TEXTURE(NAME, UV) NAME.Sample(g_samLinear, UV)

// Vertex in/out
#define IN_ATTRIBUTE(NAME) vs_input.NAME
#define INIT_POSITION float4 position : SV_POSITION;
#define OUT_ATTRIBUTE(NAME) vs_output.NAME
#define OUT_POSITION vs_output.position

// Fragment in/out
#define GET_ATRRIBUTE(NAME) fs_input.NAME
#define OUT_COLOR out_color.color

// Main functions
#define MAIN_VERTEX(VERTEX_IN, VERTEX_OUT) VERTEX_OUT mainVS(VERTEX_IN vs_input) { \
VERTEX_OUT vs_output;

#define MAIN_VERTEX_END return vs_output; \
} \

struct PixelShaderOutputFloat4
{          
	float4 color : SV_Target0;   
};
#define MAIN_FRAG(FRAG_IN) PixelShaderOutputFloat4 mainFS(FRAG_IN fs_input) \
{ \
PixelShaderOutputFloat4 out_color;

struct PixelShaderOutputUint
{          
	uint color : SV_Target0;   
};
#define MAIN_FRAG_UI(FRAG_IN) PixelShaderOutputUint mainFS(FRAG_IN fs_input) \
{ \
PixelShaderOutputUint out_color;

#define MAIN_FRAG_END return out_color; \
} \


#endif // H_LANGUAGE_DX11