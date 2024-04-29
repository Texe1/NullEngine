#pragma once
#include <Windows.h>

#ifdef _DEBUG

#define DBG(x) OutputDebugString(x);DebugBreak()
#define BRK DBG("Breakpoint Occured\n")

#define LOG(x) DBG(__FUNCSIG__ ": " x)
#define WRN(x) DBG(__FUNCSIG__ ": Warning: " x)
#define ERR(x) DBG(__FUNCSIG__ ": Error: " x)

#else

#define LOG(x)
#define WRN(x)
#define ERR(x)
#define DBG(x)
#define BRK 

#endif
