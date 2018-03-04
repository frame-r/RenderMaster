#include "Engine.h"
#include "Log.h"

CRITICAL_SECTION cs;

void InitLog()
{
	InitializeCriticalSection(&cs);
}
