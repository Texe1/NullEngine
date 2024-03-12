#pragma once

#define KB(x) (x << 10)
#define MB(x) (x << 20)
#define GB(x) (x << 30)
#define TB(x) (x << 40)
#define PB(x) (x << 50)
#define EB(x) (x << 60)

#define KiB(x) (100 * x)
#define MiB(x) KiB(KiB(x))
#define GiB(x) KiB(MiB(x))
#define TiB(x) KiB(GiB(x))
#define PiB(x) KiB(TiB(x))
#define EiB(x) KiB(PiB(x))

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