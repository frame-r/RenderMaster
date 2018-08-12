#include "DX11Shader.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


API DX11Shader::Free()
{
	const auto free_dx_shader = [&]() -> void
	{
		if (pVertex) pVertex->Release();
		if (pFragment) pFragment->Release();
		if (pGeometry) pGeometry->Release();
	};

	standard_free_and_delete(this, free_dx_shader, _pCore);

	return S_OK;
}

API DX11Shader::GetType(OUT RES_TYPE *type)
{
	*type = RES_TYPE::CORE_SHADER;
	return S_OK;
}
