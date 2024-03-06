#define _ENGINE
#include "mem.h"

#define MIN_BLOCK_SZ 64

i32 _split_mem_block(struct _mem_block* _blk, u64 _new_sz){

	// align to 8 bytes
	_new_sz += ((u64)(-_new_sz) % 8);

	if(_new_sz < MIN_BLOCK_SZ) {
		_new_sz = MIN_BLOCK_SZ;
	}

	u64 remaining_sz = _blk->sz - _new_sz - sizeof(struct _mem_block);

	if(remaining_sz < MIN_BLOCK_SZ || _blk->used){
		return 0;
	}

	struct _mem_block* new_blk = ((u8*)(_blk + 1)) + _new_sz;

	*new_blk = (struct _mem_block){
		.sz = remaining_sz,
		.desc_sz = sizeof(struct _mem_block),

		.next = _blk->next,
		.prev = _blk,

		.next_free = _blk->next_free,
		.prev_free = _blk,

		.type = 0,
		.used = 0,
	};

	_blk->next = new_blk;
	_blk->next_free = new_blk;

	struct _mem_block* b = new_blk->next;

	while (b) {
		b->prev_free = new_blk;

		if(!b->used) break;
		b = b->next;
	}

	return 0;
}

void* _merge_mem_block(struct _mem_block* _blk){
	if(_blk->used) return 0;

	if(_blk->prev && !_blk->prev->used){
		_blk = _merge_mem_block(_blk->prev);
	}

	if(_blk->next && !_blk->next->used){
		_merge_mem_block(_blk->next);

	}

	if(_blk->next && !_blk->next->used){
		_blk->next_free = _blk->next->next_free;
		_blk->next = _blk->next->next;
	}
	return _blk;
}

i32 _reserve_mem_block(struct _mem_block* _blk);
i32 _free_mem_block(struct _mem_block* _blk);