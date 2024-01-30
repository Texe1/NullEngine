#define _ENGINE
#include "mem.h"

#include "helper.h"

#include <stdlib.h>

struct _base_memory* mem;

struct _base_memory* _init_base_mem(u64 _min_sz, u64 _n_reserved_handles, u64 _n_reserved_gc_obj_refs){
	u64 alloc_sz = _min_sz + sizeof(struct _base_memory) + _n_reserved_handles * sizeof(struct _handle_table_entry) + _n_reserved_gc_obj_refs * sizeof(struct _gc_object*);

	if(alloc_sz % 8){
		alloc_sz += 8 - (alloc_sz % 8);
	}

	struct _base_memory* result = malloc(alloc_sz);
	if(!result) return NULL;
	result->base_handle_table = result+1;
	result->app_mem_start = _init_handle_table(_n_reserved_handles, result->base_handle_table);

	result->sz = alloc_sz - (sizeof(struct _base_memory) + sizeof(struct _handle_table)) - _n_reserved_handles * sizeof(struct _handle_table_entry);
	mem = result;
	return result;
}

struct memory_chunk* allocate(u64 _sz, u8 _align){

}
