#pragma once

#include "gc_structs.h"

i32 _gc_run_sweep();

i32 _gc_sweep(struct _gc*, struct _gc_object*);

i32 _gc_run_mark();

i32 _rec_gc_mark(struct _gc*, struct _gc_object*);