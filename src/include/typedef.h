#pragma once

#define KiB(x) (x << 10)
#define MiB(x) (x << 20)
#define GiB(x) (x << 30)
#define TiB(x) (x << 40)
#define PiB(x) (x << 50)
#define EiB(x) (x << 60)

#define KB(x) (100 * x)
#define MB(x) KB(KB(x))
#define GB(x) KB(MB(x))
#define TB(x) KB(GB(x))
#define PB(x) KB(TB(x))
#define EB(x) KB(PB(x))

typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned int 		u32; 
typedef unsigned long long 	u64;

typedef char 				i8;
typedef short 				i16;
typedef int 				i32;
typedef long long 			i64;

// it is recommended to use decimal for most cases, as it will stay consistent with NullEngine in future updates
typedef float 				f32;
// it is recommended to use decimal for most cases, as it will stay consistent with NullEngine in future updates
typedef double				f64;

// the default floating-point type (32-bit/single-precision)
typedef f32					decimal;