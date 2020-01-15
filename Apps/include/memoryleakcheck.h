
#ifndef _MEM_CHECK_H
#define _MEM_CHECK_H

#include "stddef.h"
#include "stdio.h"

// instead of malloc
#define os_malloc(s) dbg_malloc(s, __FILE__, __LINE__)

// instead of free
#define os_free(p) dbg_free(p)

/**
* allocation memory
*/
void *dbg_malloc(size_t elem_size, char *filename, size_t line);

/**
* deallocate memory
*/
void dbg_free(void *ptr);

/**
* show memory leake report
*/
void show_block();

#endif // _MEM_CHECK_H

