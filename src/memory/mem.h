#pragma once
#include "typedef.h"

#ifdef _ENGINE

#pragma region base_mem

struct _base_memory {
	u64 total_sz;
	u64 used_sz;

	struct _mem_block* first_block;

	struct  _gc {
		u64 active	: 1,
			marker	: 1,
			n_objs	: 62;
		void* objs;
	} gc;
	struct {
		u64 n_handles;
		void* handles;
	} handles;
	struct _dbg {
		u64 active		: 1,
			n_errors	: 63;
		void* errors;
	} dbg;
};

struct _base_mem_cfg {
	u64 sz;
	u64 n_objs;
	u64 n_handles;
};

i32 _ess_init_base_mem(struct _base_mem_cfg*);
i32 _ess_free_base_mem();

#pragma endregion

#pragma region memory_block

struct _mem_block {
	u64 sz;
	u64 used	: 1,
		type	: 15,
		desc_sz	: 48;

	struct _mem_block *next, *prev; 
	struct _mem_block *next_free, *prev_free;
};

i32 _split_mem_block(struct _mem_block*, u64 _new_sz);
void* _merge_mem_block(struct _mem_block*);

i32 _reserve_mem_block(struct _mem_block*);
i32 _free_mem_block(struct _mem_block*);

i32 _expand_mem_block(struct _mem_block*);

i32 alloc(u64 _sz);

#pragma endregion

#pragma region mem_types

struct _mem_pool {
	struct _mem_block;
	u64 block_sz;
};

#pragma endregion

#endif