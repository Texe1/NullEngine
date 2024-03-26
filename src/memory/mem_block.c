#define _ENGINE
#include "mem.h"

#define MIN_BLOCK_SZ 64

i32 _split_mem_block(struct _mem_block* _blk, u64 _new_sz){

	// align to 8 bytes
	_new_sz += ((u64)(-_new_sz) % 8);

	// assure minimum block size of MIN_BLOCK_SZ
	
	if(_new_sz < MIN_BLOCK_SZ) {
		_new_sz = MIN_BLOCK_SZ;
	}

	u64 remaining_sz = _blk->sz - _new_sz - sizeof(struct _mem_block);

	if(remaining_sz < MIN_BLOCK_SZ || _blk->used){
		return 0;
	}

	// create new block 
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

	// insert new block into linked list
	_blk->next = new_blk;
	_blk->next_free = new_blk;
	_blk->sz = _new_sz;

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

	struct _mem_block* next = _blk->next;
 
	if(!next || next->used) {
		return 0;
	}
	_merge_mem_block(next);

	_blk->next_free = next->next_free;
	_blk->next = next->next;

	if(_blk->next){
		_blk->next->prev = _blk;
	}

	_blk->sz += next->sz + next->desc_sz;

	struct _mem_block *b = next->next;
	while(b && b->used){
		b->prev_free = _blk;
		b = b->next;
	}

	return _blk;
}

i32 _reserve_mem_block(struct _mem_block* _blk){
	if(_blk->used) return 1; // TODO ERRORS

	_blk->used = 1;

	struct _mem_block *b = _blk->prev;

	do {
		b->next_free = _blk->next_free;
		b = b->prev;
	} while(b && b->used);

	b = _blk->next;

	do {
		b->prev_free = _blk->prev_free;
		b = b->next;
	} while(b && b->used);

	return 0;
}
i32 _free_mem_block(struct _mem_block* _blk){
	if(!_blk->used) return 1; // TODO ERRORS

	_blk->used = 0;
	return 0;
}