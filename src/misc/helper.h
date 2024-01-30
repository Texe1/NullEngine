#pragma once
#include <Windows.h>

#ifdef _DEBUG

#define LOG(x) OutputDebugString(x)
#define DBG(x) LOG(x);DebugBreak()
#define BRK DBG("Breakpoint Occured\n")

#else

#define LOG(x)
#define DBG(x)
#define BRK 

#endif
