#pragma once

#include "typedef.h"
#include "gc_structs.h"

// symbolizes a reference to a garbage collectible object
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


#define NULL_HANDLE ((struct handle) {0})

/*
initializes a handle table
@warning caller MUST ensure enough space for struct _handle_table and [_n] entries 
@param _n the number of allocated spaces for
@param _ptr the position where the table should be put
@returns 0
*/
i32 _init_handle_table(u64 _n, void* _ptr);

/*
creates a handle from the given objuect
@param _obj pointer to the object the handle should reference
@returns - handle to [_obj]
@returns - null-handle on failure
*/
struct handle _create_handle(struct _gc_object* _obj);

/*
TODO implementation
releases a handle so that the garbage collector can work properly
@param _h valid handle
@returns - zero on success
@returns - -1 on unhandled error
@returns - error code otherwise
*/
i32 _release_handle(struct handle _h);