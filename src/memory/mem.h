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

enum MEMORY_CHUNK_TYPE {
	MEMORY_CHUNK_TYPE_NONE,
	MEMORY_CHUNK_TYPE_TABLE,
};

struct _memory_chunk {
	u64 sz;

	struct _memory_chunk* prev;
	struct _memory_chunk* next;
	
	struct _memory_chunk* prevFree;
	struct _memory_chunk* nextFree;

	u64 free	: 1,
		fixed	: 1,
		type	: 8,
		align 	: 54;
};

// will be right after the struct _memory_chunk with type MEMORY_CHUNK_TYPE_TABLE to give more information
struct _memory_chunk_table_addon {
	u64 entry_sz;
	u64 n_entries;
	struct _memory_chunk* prev;
	struct _memory_chunk* next;
};

// the struct that defines the main memory chunk allocated during initialization
struct _base_memory {
	u64 sz;
	struct _memory_chunk* first_chunk;

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

struct _memory_chunk* _alloc(u64 _sz);
struct _memory_chunk* _calloc(u64 _sz);

struct _memory_chunk* _alloc_table(u64 _n, u64 _sz);
struct _memory_chunk* _calloc_table(u64 _n, u64 _sz);

i32 _rec_mem_chunk_table_get(struct _memory_chunk* _mem, u64 _idx, void* _ret);
i32 _rec_mem_chunk_table_set(struct _memory_chunk* _mem, u64 _idx, void* _data);
i32 _rec_mem_chunk_table_bulk_get(struct _memory_chunk* _mem, u64 _idx, u64 _len, void* _ret);
i32 _rec_mem_chunk_table_bulk_set(struct _memory_chunk* _mem, u64 _idx, u64 _len, void* _data);

i32 _free(struct _memory_chunk* _chunk);

#endif