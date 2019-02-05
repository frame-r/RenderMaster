#pragma once
#include "Common.h"

class RenderTarget : public BaseResource<IRenderTarget>
{
	unique_ptr<ICoreRenderTarget> _coreRenderTarget;

public:
	RenderTarget(unique_ptr<ICoreRenderTarget> renderTaget);

	API_RESULT GetCoreRenderTarget(ICoreRenderTarget **renderTargetOut) override;
	API_RESULT SetColorTexture(uint slot, ITexture *tex) override { _coreRenderTarget->SetColorTexture(slot, tex); return S_OK; }
	API_RESULT SetDepthTexture(ITexture *tex) override { _coreRenderTarget->SetDepthTexture(tex); return S_OK; }
	API_RESULT UnbindColorTexture(uint slot) override { _coreRenderTarget->UnbindColorTexture(slot); return S_OK; }
	API_RESULT UnbindAll() override { _coreRenderTarget->UnbindAll(); return S_OK; }
};
