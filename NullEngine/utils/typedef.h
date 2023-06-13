#pragma once

typedef unsigned char  u8;
typedef unsigned short u16; 
typedef unsigned int u32;
typedef unsigned long long u64;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

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

	#if DBG_LVL > 1
		#define WRN(x) x

		#if DBG_LVL > 2
			#define INF(x) x
		#else // DBG_LVL > 2
			#define INF(x) 
		#endif // DBG_LVL > 2

	#else // DBG_LVL > 1
		#define WRN(x) 
		#define INF(x) 
	#endif // DBG_LVL > 1

#else // DBG_LVL > 0
	#define ERR(x) 
	#define WRN(x) 
	#define INF(x) 
#endif // DBG_LVL > 0