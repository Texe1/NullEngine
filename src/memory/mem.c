#define _ENGINE
#include "mem.h"

#include "helper.h"

#include <stdlib.h>

struct _base_memory* mem;

struct _base_memory* _ess_init_base_mem(struct _base_memory_create_info* _info){

	u64 sz_handle_table = sizeof(struct _handle_table) + _info->n_handles * sizeof(struct _handle_table_entry);
	u64 sz_obj_array	= _info->n_objs * sizeof(struct _gc_object);
	u64 sz_field_array	= _info->n_obj_fields * sizeof(struct _gc_object_field);

	u64 addSize = sizeof(struct _base_memory) + sz_handle_table + sz_obj_array + sz_field_array;

	u64 alloc_sz = _info->plain_sz + addSize;

	if(alloc_sz % 8){
		alloc_sz += 8 - (alloc_sz % 8);
	}

	struct _base_memory* result = malloc(alloc_sz);
	if(!result) return NULL;

	mem = result;
	*mem = (struct _base_memory){
		.sz = alloc_sz - addSize,
		.base_handle_table = mem + 1,
		.gc = {
			.active = 1,
			.mark_bit = 0,
			.max_n_fields = _info->n_obj_fields,
			.max_n_obj = _info->n_obj_fields,
			.n_fields = 0,
			.n_objects = 0,
		},
	};
	mem->gc.objs = ((u8*)mem->base_handle_table) + sz_handle_table;
	mem->gc.obj_fields = ((u8*)mem->gc.objs) + sz_obj_array;

	mem->app_mem_start = ((u8*)mem->gc.obj_fields) + sz_field_array;
	_init_handle_table(_info->n_handles, result->base_handle_table);
	mem = result;
	return result;
}

struct memory_chunk* allocate(u64 _sz, u8 _align){

}
