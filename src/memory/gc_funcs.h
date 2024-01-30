#pragma once

#include "gc_structs.h"
#include "handle.h"

/*
TODO function descriptions
*/
i32 _gc_run_sweep();

/*
TODO function descriptions
*/
i32 _gc_sweep(struct _gc*, struct _gc_object*);

/*
TODO function descriptions
*/
i32 _gc_run_mark();

/*
TODO function descriptions
*/
i32 _rec_gc_mark(struct _gc*, struct _gc_object*);

/*
TODO function descriptions
*/
struct handle _create_object(int type);