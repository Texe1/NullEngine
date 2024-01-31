#define _ENGINE
#include "handle.h"

#include <string.h>
#include "mem.h"

extern struct _base_memory* mem;

i32 _init_handle_table(u64 _n, void* _ptr){
	struct _handle_table* table = _ptr;
	void* data = table + 1;

	memset(data, 0, _n * sizeof(struct _handle_table_entry));

	table->nFree = _n;
	*table = (struct _handle_table) {
		.nFree = _n,
		.nUsed = 0,
		.nMax = _n,
	};

	return 0;
}


struct handle _create_handle(struct _gc_object* _obj){
	struct _handle_table* tbl = mem->base_handle_table;

	if(!tbl || !_obj || !tbl->nFree) return (struct handle){0};
	struct _handle_table_entry* handles = tbl + 1;

	for(u64 i = 0; i < tbl->nMax; i++){
		if(handles[i].used) continue;

		handles[i].used = 1;
		handles[i].ref = _obj;
		_obj->ref_cnt++;

		tbl->nFree--;
		tbl->nUsed++;

		struct handle h = {
			.counter = handles[i].counter,
			.idx = i,
		}; 
		return h;
	}

	return (struct handle){0};
}