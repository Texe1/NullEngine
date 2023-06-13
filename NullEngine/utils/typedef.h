#pragma once

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16; 
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

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