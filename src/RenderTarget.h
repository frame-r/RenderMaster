#pragma once
#include "Common.h"

class RenderTarget : public IRenderTarget
{
	ICoreRenderTarget *_coreRenderTarget = nullptr;

public:
	RenderTarget(ICoreRenderTarget *renderTaget) { _coreRenderTarget = renderTaget; }
	virtual ~RenderTarget();

	API GetCoreRenderTarget(ICoreRenderTarget **renderTargetOut) override;
	API SetColorTexture(uint slot, ITexture *tex) override { _coreRenderTarget->SetColorTexture(slot, tex); return S_OK; }
	API SetDepthTexture(ITexture *tex) override { _coreRenderTarget->SetDepthTexture(tex); return S_OK; }
	API UnbindColorTexture(uint slot) override { _coreRenderTarget->UnbindColorTexture(slot); return S_OK; }
	API UnbindAll() override { _coreRenderTarget->UnbindAll(); return S_OK; }

	RUNTIME_ONLY_RESOURCE_HEADER
};
