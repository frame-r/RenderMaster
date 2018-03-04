#include "Engine.h"
#include "Log.h"
#include "LoadMeshTask.h"

using namespace RENDER_MASTER;

ICore* pCore;

class LogWindowSubscriber : public ILogEventSubscriber
{
	const char *_name;
public:
	LogWindowSubscriber(const char *name) : _name(name) {}
	API Call(const char* pStr, LOG_TYPE type) override
	{
		std::cout << "Object  " << _name << " catch form engine message \"" << pStr << "\"\n";
		return S_OK;
	}
};

int main(int argc, char *argv[])
{
	
	InitLog();

	if (GetEngine(pCore))
	{
		LogWindowSubscriber o1("object1");
		LogWindowSubscriber o2("object2");
		ILogEvent *ev;

		pCore->GetLogPrintedEv(ev);

		ev->Subscribe(&o1);
		ev->Subscribe(&o2);

		HWND h;
		pCore->Init(INIT_FLAGS::IF_SELF_WINDOW, h);

		ev->Unsubscribe(&o1);
		ev->Unsubscribe(&o2);

		/*
		LoadMeshTask task;
		task.Start();
		
		LoadMeshTask task1;
		task1.Start();

		HANDLE hTimer = CreateEvent(NULL, FALSE, FALSE, NULL);
		
		int nCount = 0;
		while (nCount < 10)
		{
			WaitForSingleObject(hTimer, 1000);
			PrintLn("1 sec.");
			nCount++;
		}
		
		CloseHandle(hTimer);
		*/


		pCore->Log("hello!", LOG_TYPE::LT_NORMAL);


		pCore->CloseEngine();

		FreeEngine(pCore);
	}
	
	getchar();

	return 0;
}



