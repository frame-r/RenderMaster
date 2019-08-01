
#include "vertex.hlsli"


#ifdef ENG_SHADER_PIXEL

	cbuffer const_buffer_model_parameters
	{
		uint id;
	};

	uint mainFS(VS_OUT fs_input) : SV_Target0
	{
		return id;
	}

#endif
