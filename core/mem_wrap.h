#ifndef __MEM_WRAP_H__
#define __MEM_WRAP_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include "dmt.h"
extern unsigned int g_mem_mem_allocs_count;

void mem_free(void *ptr);

#ifdef USE_DMT
#define mem_alloc(size) dmt_malloc(size)
#else
void * mem_alloc(size_t size);
#endif

void mem_wrap_print_mallocs(void);


char * mem_strdup(const char *s);
#ifdef __cplusplus
}
#endif

#endif
