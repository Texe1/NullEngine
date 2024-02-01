#include "mem.h"
#include "helper.h"
#include <stdlib.h>

extern struct _base_memory* mem;

struct _memory_chunk* _alloc_table(u64 _n, u64 _sz){
	u64 alloc_sz = _n * _sz + sizeof(struct _memory_chunk_table_addon);
	struct _memory_chunk* chunk = _alloc(alloc_sz);
	if(!chunk) return NULL;
	chunk->type = MEMORY_CHUNK_TYPE_TABLE;
	struct _memory_chunk_table_addon* tbl = (chunk + 1);
	tbl->entry_sz = _sz;
	tbl->n_entries = _n;

	tbl->prev = NULL,
	tbl->next = NULL;

	return chunk;
}

struct _memory_chunk* _calloc_table(u64 _n, u64 _sz){
	struct _memory_chunk* chunk = _alloc_table(_n, _sz);
	if(!chunk) return NULL;
	struct _memory_chunk_table_addon* tbl = chunk + 1;
	void* data = tbl + 1;
	memset(data, 0, _n * _sz);

	return chunk;
}

void* _rec_mem_chunk_table_get_ptr(struct _memory_chunk* _mem, u64 _idx){
	if(!mem) return NULL;

	struct _memory_chunk_table_addon* tbl = _mem + 1;
	if(_idx >= tbl->n_entries){
		return _rec_mem_chunk_table_get_ptr(_mem->next, _idx - tbl->n_entries);
	}

	return ((u8*)(tbl + 1)) + _idx * tbl->entry_sz;
}

i32 _rec_mem_chunk_table_get(struct _memory_chunk* _mem, u64 _idx, void* _ret){
	void* entry = _rec_mem_chunk_table_get_ptr(_mem, _idx);
	if(!entry) return -1;
	struct _memory_chunk_table_addon* tbl = _mem + 1;
	memcpy(_ret, entry, tbl->entry_sz);
	return 0;
}

i32 _rec_mem_chunk_table_set(struct _memory_chunk* _mem, u64 _idx, void* _data){
	void* entry = _rec_mem_chunk_table_get_ptr(_mem, _idx);
	if(!entry) return -1;
	struct _memory_chunk_table_addon* tbl = _mem + 1;
	memcpy(entry, _data, tbl->entry_sz);
	return 0;
}

i32 _rec_mem_chunk_table_bulk_set(struct _memory_chunk* _mem, u64 _idx, u64 _len, void* _data){
	if(!_len || !mem) return 1;
	struct _memory_chunk_table_addon* tbl = _mem + 1;

	if(_idx >= tbl->n_entries){
		return _rec_mem_chunk_table_bulk_set(tbl->next, _idx - tbl->n_entries, _len, _data);
	}

	u64 this_len = _len;

	if(_idx + _len > tbl->n_entries){
		this_len = tbl->n_entries - _idx;
		u64 next_len = _len - this_len;
		void* next_data = ((u8*)_data) + (tbl->entry_sz * this_len);
		_rec_mem_chunk_table_bulk_set(tbl->next, 0, next_len, next_data);
	}

	u64 _sz = this_len * tbl->entry_sz;
	u8* tbl_data = tbl + 1;
	tbl_data += tbl->entry_sz * _idx;

	memcpy(tbl_data, _data, tbl->entry_sz * _len);

	return 0;
}

i32 _rec_mem_chunk_table_bulk_get(struct _memory_chunk* _mem, u64 _idx, u64 _len, void* _ret){
if(!_len || !mem) return 1;
	struct _memory_chunk_table_addon* tbl = _mem + 1;

	if(_idx >= tbl->n_entries){
		return _rec_mem_chunk_table_bulk_get(tbl->next, _idx - tbl->n_entries, _len, _ret);
	}

	if(_idx + _len > tbl->n_entries){
		u64 this_len = tbl->n_entries - _idx;
		u64 next_len = _len - this_len;
		void* next_data = ((u8*)_ret) + (tbl->entry_sz * this_len);

		_rec_mem_chunk_table_bulk_get(tbl, _idx, this_len, _ret);
		return _rec_mem_chunk_table_bulk_get(tbl->next, 0, next_len, next_data);
	}

	u64 _sz = _len * tbl->entry_sz;
	u8* tbl_data = tbl + 1;
	tbl_data += tbl->entry_sz * _idx;

	memcpy(_ret, tbl_data, tbl->entry_sz * _len);

	return 0;
}