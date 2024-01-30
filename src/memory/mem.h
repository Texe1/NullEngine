#pragma once

#include "typedef.h"
#include "handle.h"
#include "gc_structs.h"

#define KiB(n) (1024 *n)
#define MiB(n) KiB(KiB(n))
#define GiB(n) KiB(MiB(n))
#define TiB(n) KiB(GiB(n))
#define PiB(n) KiB(TiB(n))
#define EiB(n) KiB(PiB(n))

#define KB(n) (1000 *n)
#define MB(n) KB(KB(n))
#define GB(n) KB(MB(n))
#define TB(n) KB(GB(n))
#define PB(n) KB(TB(n))
#define EB(n) KB(PB(n))

#ifdef _ENGINE

struct memory_chunk {
	u64 sz;

	struct memory_chunk* prev;
	struct memory_chunk* next;
	
	struct memory_chunk* prevFree;
	struct memory_chunk* nextFree;

	u64 free	: 1,
		type	: 8,
		align 	: 16;
};

struct _base_memory {
	u64 sz;
	struct memory_chunk* app_mem_start;
	
	struct _handle_table* base_handle_table;
	struct _gc gc;
};



/*
initializes base memory
@warning if this fails, NullEngine will crash, since there is not enough memory to allocate
@param  _min_sz the minimum freely available size in bytes
@param _n_reserved_handles amount of spaces for handles(references) that is prereserved (does not count towards [_min_sz])
@param _n_reserved_gc_obj_refs amount of spaces for the object table (for garbage collector) that is reserved (does not count towards [_min_sz])
*/
struct _base_memory* _init_base_mem(u64 _min_sz, u64 _n_reserved_handles, u64 _n_reserved_gc_obj_refs);

#endif