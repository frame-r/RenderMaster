#pragma once
#include "Engine.h"

using namespace RENDER_MASTER;

extern ICore* pCore;

class LoadMeshTask : IProgressSubscriber
{
	IResourceManager *pResMan;
	IMesh *pMesh;
	DWORD m_ThreadId;

	API ProgressChanged(uint i) override;

	BOOL ClassThreadProc();
	static DWORD WINAPI RealThreadProc(void* pv);

public:
	
	void Start();
};

