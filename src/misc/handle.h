#pragma once

#include "typedef.h"

struct handle {
	u64 counter;
	u64 idx;
};

struct _handle_table {
	u64 nUsed;
	u64 nFree;
	u64 nMax;
};

struct _handle_table_entry{
	u64 counter	: 63,
		used	: 1;
	struct _gc_object* ref;
};

/*
initializes a handle table
@warning caller MUST ensure enough space for struct _handle_table and [_n] entries 
@param _n the number of allocated spaces for
@param _ptr the position where the table should be put
@returns address of next byte after the table
*/
void* _init_handle_table(u64 _n, void* _ptr);