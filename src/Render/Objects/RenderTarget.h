#pragma once
#include "Common.h"

class RenderTarget : public BaseResource<IRenderTarget>
{
	unique_ptr<ICoreRenderTarget> _coreRenderTarget;

public:
	RenderTarget(unique_ptr<ICoreRenderTarget> renderTaget);

	API GetCoreRenderTarget(ICoreRenderTarget **renderTargetOut) override;
	API SetColorTexture(uint slot, ITexture *tex) override { _coreRenderTarget->SetColorTexture(slot, tex); return S_OK; }
	API SetDepthTexture(ITexture *tex) override { _coreRenderTarget->SetDepthTexture(tex); return S_OK; }
	API UnbindColorTexture(uint slot) override { _coreRenderTarget->UnbindColorTexture(slot); return S_OK; }
	API UnbindAll() override { _coreRenderTarget->UnbindAll(); return S_OK; }
};
