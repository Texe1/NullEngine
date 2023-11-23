#pragma once

#include <stdint.h>

typedef unsigned char u8;
typedef unsigned short u16; 
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

#define KB(x) (x << 10)
#define MB(x) (x << 20)
#define GB(x) (x << 30)
#define TB(x) (x << 40)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#ifdef _DEBUG
#ifndef DBG_LVL
#define DBG_LVL 3
#endif // !DBG_LVL
#else  // _DEBUG
#define DBG_LVL 0
#endif // _DEBUG

#if DBG_LVL > 0
	#define ERR(x) x
	#define LOGERR printf

	#if DBG_LVL > 1
		#define WRN(x) x
		#define LOGWRN printf

		#if DBG_LVL > 2
			#define INF(x) x
			#define LOGINF printf
		#else // DBG_LVL > 2
			#define INF(x) 
			#define LOGINF(x) 
		#endif // DBG_LVL > 2

	#else // DBG_LVL > 1
		#define WRN(x) 
		#define LOGWRN(x) 

		#define INF(x) 
		#define LOGINF(x) 
	#endif // DBG_LVL > 1

#else // DBG_LVL > 0
	#define ERR(x) 
	#define LOGERR(x)
	#define WRN(x) 
	#define LOGWRN(x) 
	#define INF(x) 
	#define LOGINF(x) 
#endif // DBG_LVL > 0