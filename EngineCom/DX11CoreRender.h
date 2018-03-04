#pragma once
#include "Common.h"

class DX11CoreRender : public ICoreRender
{
public:

	DX11CoreRender();
	~DX11CoreRender();

	// ISubSystem
	API GetName(const char *&pTxt) override;

	API Init(WinHandle& handle) override;
	API Clear() override;
	API Free() override;

};

