#define _ENGINE
#include "gc_structs.h"
#include "gc_funcs.h"

#include "mem.h"
#include "handle.h"

extern struct _base_memory* mem;

i32 _gc_run_sweep(){
	struct _gc* gc = &mem->gc;

	if(!gc->active) return 1;

	u64 n = 0;
	for(u64 i = 0; i < gc->max_n_obj && n < gc->n_objects; i++){
		struct _gc_object* obj = &(gc->objs[i]);

		if(obj->gc_active && obj->gc_mark == gc->mark_bit){
			i32 result = _gc_sweep(gc, obj);
			if(result){
				//TODO ERRORS
			}
		}
	}

	gc->mark_bit = !(gc->mark_bit);

	return 0;
}

i32 _gc_sweep(struct _gc* _gc, struct _gc_object* _obj){
	struct _gc_object_field* fields = _obj->fields;

	for(u64 i = 0; i < _obj->nFields; i++){
		if(fields[i].type == GC_OBJECT_FIELD_NULL || !(fields[i].ref_count)) return -1;

		fields[i].ref_count--;
		if(!fields[i].ref_count){
			fields[i].type = GC_OBJECT_FIELD_NULL;
		}
	}
	return 0;
}

i32 _gc_run_mark(){

	struct _gc* gc = &(mem->gc);

	if(!(gc->active)) return 1;

	struct _handle_table* table = mem->base_handle_table;
	struct _handle_table_entry* handles = table + 1;

	int n = 0;
	for(u64 i; i < table->nMax && n < table->nUsed; i++){
		if(handles[i].used){
			_rec_gc_mark(gc, handles[i].ref);
			n++;
		}
	}

	return 0;
}

i32 _rec_gc_mark(struct _gc* _gc, struct _gc_object* _obj){
	if(_obj->magicNum != GC_MAGIC_NUM) return 1;

	if(_obj->gc_mark == _gc->mark_bit) return 0;

	_obj->gc_mark = _gc->mark_bit;

	struct _gc_object_field* fields = _obj->fields;

	for(int i = 0; i < _obj->nFields; i++){
		if(fields[i].type != GC_OBJECT_FIELD_REF) continue;
		if(!fields[i].isArray){
			_rec_gc_mark(_gc, fields[i].data);
			continue;
		}

		struct _gc_object** ref_list = fields[i].data;

		for(u32 i = 0; i < fields[i].sz; i++){
			_rec_gc_mark(_gc, ref_list[i]);
		}
	}
}