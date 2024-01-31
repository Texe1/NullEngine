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

	mem->first_chunk = ((u8*)mem->gc.obj_fields) + sz_field_array;

	*(mem->first_chunk) = (struct _memory_chunk){ 0 };
	mem->first_chunk->free = 1;
	mem->first_chunk->sz = mem->sz;

	_init_handle_table(_info->n_handles, result->base_handle_table);
	mem = result;
	return result;
}

i32 _rec_update_chunk(struct _memory_chunk* _chunk){
	if(!_chunk) return 0;

	if(_chunk->next){
		if(_chunk->nextFree != _chunk->next->nextFree){
			_chunk->nextFree = _chunk->next->nextFree;
			if(_chunk->next->free){
				_chunk->nextFree = _chunk->next;
			}
			_rec_update_chunk(_chunk->prev);
		}
	}

	if(_chunk->prev){
		if(_chunk->prevFree != _chunk->prev->prevFree){
			_chunk->nextFree = _chunk->prev->prevFree;
			if(_chunk->prev->free){
				_chunk->prevFree = _chunk->prev;
			}
			_rec_update_chunk(_chunk->next);
		}

		if(!_chunk->prev->next){
			DBG("MEMORY CHUNKS CORRUPTED");
			return -1;
		}
		_chunk->prev = _chunk->prev->next;
	}

	return 0;
}

/*
TODO more sophisticated selection
*/
struct _memory_chunk* _get_free_mem_chunk(u64 _sz){
	struct _memory_chunk* c = mem->first_chunk;
	if(!(c->free)) c = c->nextFree;

	while(c){
		if(c->sz > _sz){
			return c;
		}
		c = c->nextFree;
	}

	return NULL;
}

#define MIN_MEM_CHUNK_SIZE 64

i32 _split_mem_chunk(struct _memory_chunk* _chunk, u64 _sz){
	if(!(_chunk->free)) return 1;
	if(_chunk->sz - _sz - sizeof(struct _memory_chunk) <= MIN_MEM_CHUNK_SIZE) return 0;

	struct _memory_chunk* new_chunk = ((u8*)(_chunk + 1)) + _sz;
	*new_chunk = (struct _memory_chunk){
		.sz = _chunk->sz - _sz - sizeof(struct _memory_chunk),
		.type = 0,
		.free = 1,
		
		.nextFree = _chunk->nextFree,
		.prevFree = (_chunk->free) ? _chunk : _chunk->prevFree,

		.next = _chunk->next,
		.prev = _chunk,

		.align = 0,
	};

	_chunk->sz = _sz;
	_chunk->next = new_chunk;
	_chunk->nextFree = new_chunk;

	if(new_chunk->next){
		new_chunk->next->prev = new_chunk;
		
		struct _memory_chunk* c = new_chunk->next;
		while(c){
			c->prevFree = new_chunk;
			if(c->free) break;
			c = c->next;
		}
	}

	return 0;
}

i32 _reserve_mem_chunk(struct _memory_chunk* _chunk){
	if(!(_chunk->free)) return 1;

	_chunk->free = 0;

	struct _memory_chunk* c = _chunk->next;

	while(c){
		c->prevFree = _chunk->prevFree;
		if(c->free) break;
		c = c->next;
	}
	
	c = _chunk->prev;

	while(c){
		c->nextFree = _chunk->nextFree;
		if(c->free) break;
		c = c->prev;
	}

	return 0;
}

i32 _rec_try_merge(struct _memory_chunk* _chunk){
	if(!(_chunk) || !(_chunk->next)
		|| !(_chunk->free) || !(_chunk->next->free)) return 1;

	_rec_try_merge(_chunk->next);

	_chunk->sz += _chunk->next->sz + sizeof(struct _memory_chunk);

	_chunk->nextFree = _chunk->next->nextFree;
	_chunk->next = _chunk->next->next;
	if(_chunk->next){
		_chunk->next->prev = _chunk;
		
		struct _memory_chunk* c = _chunk->next;
		while(c) {
			c->prevFree = _chunk;
			if(c->free) break;
			c = c->next;
		}
	}
	return 0;
}

struct _memory_chunk* _alloc(u64 _sz){
	struct _memory_chunk* c = _get_free_mem_chunk(_sz);
	if(!c) return NULL;
	_split_mem_chunk(c, _sz);
	_reserve_mem_chunk(c);
	return c;
}

i32 _free(struct _memory_chunk* _chunk){
	_chunk->free = 1;
	if(_chunk->prev){
		_rec_try_merge(_chunk->prev);
	}else {
		_rec_try_merge(_chunk);
	}
	return 0;
}
