#define _ENGINE
#include "handle.h"

#include <string.h>
#include "mem.h"

extern struct _base_memory* mem;

i32 _ess_init_handle_table(u64 _n){
	if(mem->handle_table.tbl) return 1;

	struct _memory_chunk* chunk = _calloc_table(_n, sizeof(struct _handle_table_entry));
	if(!chunk) return -1;

	chunk->fixed = 1;

	mem->handle_table = (struct _handle_table) {
		.tbl  = chunk,
		.nMax = _n,
		.nFree = _n,
		.nUsed = 0,
	};

	return 0;
}


struct handle _create_handle(struct _gc_object* _obj){
	struct _handle_table* handle_table = &(mem->handle_table);


	if(!handle_table->tbl || !_obj || !handle_table->nFree) return (struct handle){0};

	struct _handle_table_entry entry;

	for(u64 i = 0; i < handle_table->nMax; i++){
		// TODO low level access function for only getting pointer of entry
		if(_rec_mem_chunk_table_get(handle_table->tbl, i, &entry)) break;
		if(entry.used) continue;

		entry.used = 1;
		entry.ref = _obj;
		_obj->ref_cnt++;

		handle_table->nFree--;
		handle_table->nUsed++;

		struct handle h = {
			.counter = entry.counter,
			.idx = i,
		}; 

		_rec_mem_chunk_table_set(handle_table->tbl, i, &entry);
		return h;
	}

	return (struct handle){0};
}