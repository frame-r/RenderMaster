#pragma once
#include <iostream>

extern CRITICAL_SECTION cs;

void InitLog();

template<typename T>
inline void Print(T i)
{
	EnterCriticalSection(&cs);
	std::cout << i;
	LeaveCriticalSection(&cs);
}

template<typename T>
inline void PrintLn(T i)
{
	EnterCriticalSection(&cs);
	std::cout << i << std::endl;
	LeaveCriticalSection(&cs);
}
