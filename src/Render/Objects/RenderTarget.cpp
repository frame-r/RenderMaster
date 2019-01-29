#pragma once
#include "Pch.h"
#include "Core.h"
#include "RenderTarget.h"
#include "ResourceManager.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

RUNTIME_ONLY_RESOURCE_IMPLEMENTATION(RenderTarget, _pCore, RemoveRuntimeRenderTarget)

RenderTarget::RenderTarget(unique_ptr<ICoreRenderTarget> renderTaget)
{
	_coreRenderTarget = std::move(renderTaget);
}

API RenderTarget::GetCoreRenderTarget(ICoreRenderTarget **renderTargetOut)
{
	*renderTargetOut = _coreRenderTarget.get();
	return S_OK;
}
