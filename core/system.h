#ifndef __SYSTEM_H__
#define __SYSTEM_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdint.h>
#define UNDEFINED_ID -1

typedef struct system_s system_t;

#include "event.h"

typedef void (*system_init_func_t)(events_map_t *event_map, system_t *system);

struct system_s {
    int id;
    const char *name;
    uint32_t *local_events_map;
    uint32_t *local_components_map;
};


void system_init_local_events_map(system_t *system, size_t size);
void system_init_local_components_map(system_t *system, size_t size);
void system_init(system_t *system, const char *name);
void system_clean(system_t *system);

#ifdef __cplusplus
}
#endif

#endif
