#pragma once 

#include "typedef.h"

#define _ENGINE
#ifdef _ENGINE

enum GC_OBJECT_FIELD_TYPES {
	GC_OBJECT_FIELD_NULL 	= 0x0,
	GC_OBJECT_FIELD_REF 	= 0x1,
	GC_OBJECT_FIELD_1B 		= 0x2,
	GC_OBJECT_FIELD_2B	 	= 0x3,
	GC_OBJECT_FIELD_4B	 	= 0x4,
	GC_OBJECT_FIELD_8B	 	= 0x5,
	GC_OBJECT_FIELD_16B		= 0x6,
	GC_OBJECT_FIELD_STRUCT	= 0x7,
};


// TODO Select proper number
#define GC_MAGIC_NUM 0x01234567

struct _gc {
	u8 	active, 	: 1,
		mark_bit	: 1; // which state currently counts as marked (switches after every gc run)

	u64 n_objects, max_n_obj;
	struct _gc_object* objs;

	u64 n_fields, max_n_fields;
	struct _gc_object_field* obj_fields;
};

// base struct for garbage collectible object
struct _gc_object {

	// If this number is not MAGIC_NUM, this is not an object
	u32 magicNum;

	u32 type		: 30,
		gc_mark		: 1,
		gc_active	: 1;

	u64 sz;
	u64 ref_cnt;
	u64 nFields;
		
	struct _gc_object_field* fields;
	void* data;
};

struct _gc_object_field {
	u16 isArray		: 1,
		type		: 3,
		struct_sz	: 12;
	u16 ref_count;

	/*
	size of array for valid array field
	number of consecutive free _gc_object_field structs, when invalid field
	*/
	u32 sz;
	void* data;
};

// blueprint struct for garbage collectible object
struct _gc_object_type{
	u64 nFields;
	struct _gc_field_prototype {
		u16 isArray		: 1,
		type		: 3,
		struct_sz	: 12;
		u32 sz;
	}* fields;
};

#endif