#pragma once
#include <Windows.h>

#define LOG(x) OutputDebugString(x)
#define DBG(x) LOG(x);DebugBreak()
#define BRK DBG("Breakpoint Occured\n")
