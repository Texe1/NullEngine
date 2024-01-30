#define _ENGINE
#include "handle.h"

#include <string.h>

void* _init_handle_table(u64 _n, void* _ptr){
	struct _handle_table* table = _ptr;
	void* data = table + 1;

	memset(data, 0, _n * sizeof(struct _handle_table_entry));

	table->nFree = _n;
	*table = (struct _handle_table) {
		.nFree = _n,
		.nUsed = 0,
		.nMax = _n,
	};
	
	return ((char*)data) + _n * sizeof(struct _handle_table_entry);
}