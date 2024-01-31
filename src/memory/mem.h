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

// the struct that defines the main memory chunk allocated during initialization
struct _base_memory {
	u64 sz;
	struct memory_chunk* app_mem_start;
	
	struct _handle_table* base_handle_table;
	struct _gc gc;
};

struct _base_memory_create_info {
	u64 plain_sz;
	u64 n_handles;
	u64 n_objs;
	u64 n_obj_fields;
};



/*
initializes base memory
@warning if this fails, NullEngine will crash, since there is not enough memory to allocate
@param _info self explanatory
*/
struct _base_memory* _ess_init_base_mem(struct _base_memory_create_info* _info);

#endif