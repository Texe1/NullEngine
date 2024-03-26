#define _ENGINE
#include "mem.h"

#include <stdio.h>
#include <stdlib.h>

struct _base_memory mem;

i32 _ess_init_base_mem(struct _base_mem_cfg* _cfg){

	u64 sz = _cfg->sz + (((u64)(-_cfg->sz)) % 8);

	mem = (struct _base_memory){
		.total_sz = sz,
		.used_sz = 0,
		.first_block = calloc(sz / 8, 8),
		
		.gc = {
			.active = 1,
			.marker = 0,
			.n_objs = 0,
			.objs = NULL // TODO
		},
		.dbg = { // TODO
			.active = 0,
			.n_errors = 0,
			.errors = NULL
		}
	};

	*(mem.first_block) = (struct _mem_block) {
		.sz = sz - sizeof(struct _mem_block),
		.desc_sz = sizeof(struct _mem_block),
		.type = 0,
		.used = 0,
		.next = NULL,
		.prev = NULL,
		.next_free = NULL,
		.prev_free = NULL,
	};

	return 0;
}

//TODO edit once there can be additional base memory sections (when we run out of memory)
i32 _ess_free_base_mem(){
	free(mem.first_block);
}