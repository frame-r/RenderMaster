#include "LoadMeshTask.h"
#include "Log.h"


API LoadMeshTask::ProgressChanged(uint i)
{
	char buf[20];
	sprintf_s(buf, "loading mesh %i %%", i);

	PrintLn(buf);

	return S_OK;
}

void LoadMeshTask::Start()
{
	
	pCore->GetSubSystem((ISubSystem *&)pResMan, SUBSYSTEM_TYPE::ST_RESOURCE_MANAGER);
	
	HANDLE m_hThread = ::CreateThread(NULL,		// Default security  
			0,									// Default stack size  
			RealThreadProc,
			(void*)this,
			0,  
			&m_ThreadId);
}

DWORD WINAPI LoadMeshTask::RealThreadProc(void* pv)
{
	LoadMeshTask* pFree = reinterpret_cast<LoadMeshTask*>(pv);
	return pFree->ClassThreadProc();
}

BOOL LoadMeshTask::ClassThreadProc()
{
	pResMan->LoadModel(pMesh, "", (IProgressSubscriber *)this);

	uint v;
	pMesh->GetVertexCount(v);

	Print("mesh loaded vertexes = ");
	PrintLn(v);

	return TRUE;
}
