#ifndef __SYSTEM_H__
#define __SYSTEM_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "macros.h"
#include "int_list.h"
#include <stdlib.h>
#include <stdint.h>

#define NOT_INITIALIZED ""
#define UNDEFINED_ID -1

typedef struct system_s system_t;

//#include "event.h"

struct system_s {
    const char *name;
    uint32_t *local_events_map;
    uint32_t *local_components_map;
    MAYBE(void *) data;
    MAYBE_FUNC(free_callback_t) data_free;
};


void system_init_local_events_map(system_t *system, size_t size);
void system_init_local_components_map(system_t *system, size_t size);
system_t * system_new(void);
void system_free(system_t *system);

#ifdef __cplusplus
}
#endif

#endif
