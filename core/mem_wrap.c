#include "mem_wrap.h"
#include "dmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

unsigned int g_mem_mem_allocs_count;

#ifndef USE_DMT
void * mem_alloc(size_t size) {
    void *ptr = malloc(size);
    assert(ptr != NULL);
    ++g_mem_mem_allocs_count;
    return ptr;
    return dmt_malloc(size);
}

#endif


void mem_free(void *ptr) {
#ifndef USE_DMT
    --g_mem_mem_allocs_count;
    free(ptr);
#else
    dmt_free(ptr);
#endif
}

void mem_wrap_print_mallocs(void) {
#ifndef USE_DMT
    printf("\nUnfreed mallocs: %d", g_mem_mem_allocs_count);
#else
    dmt_dump(stdout);
#endif
}

char * mem_strdup(const char *s) {
    char *t = mem_alloc(strlen(s));
    return t;
}
