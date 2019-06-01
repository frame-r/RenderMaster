
struct VS_OUT
{
	float4 position : SV_POSITION;
};
	
#ifdef ENG_SHADER_VERTEX

	cbuffer vertex_transformation_parameters
	{
		float4x4 MVP;
	};

	struct VS_IN
	{
		float4 PositionIn : POSITION0;
	};

	VS_OUT mainVS(VS_IN vs_input)
	{
		VS_OUT vs_output;
		vs_output.position = mul(MVP, vs_input.PositionIn);

		return vs_output;
	}

#elif ENG_SHADER_PIXEL

	cbuffer const_buffer_material_parameters
	{
		float4 main_color;
	};

	float4 mainFS() : SV_Target0
	{
		return main_color;
	}

#endif
