#include "Engine.h"

int main(int argc, char *argv[])
{
	ICore* pCore;

	if (GetEngine(pCore))
	{
		pCore->StartEngine();
		pCore->CloseEngine();

		FreeEngine(pCore);
	}
	
	getchar();

	return 0;
}



