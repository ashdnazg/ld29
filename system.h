#ifndef __SYSTEM_H__
#define __SYSTEM_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#define UNDEFINED_ID -1

typedef struct system_s system_t;

#include "event.h"

typedef void (*system_init_func_t)(events_map_t *event_map, system_t *system);

struct system_s {
    int id;
    const char *name;
    size_t *local_events_map;
};


void system_init_local_map(system_t *system, size_t size);
void system_init(system_t *system, const char *name);
void system_clean(system_t *system);

#ifdef __cplusplus
}
#endif

#endif
