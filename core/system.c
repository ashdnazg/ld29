#include "system.h"
#include "mem_wrap.h"
#include <stdio.h>

void system_init(system_t *system, const char *name) {
    system->id = UNDEFINED_ID;
    system->name = name;
    system->local_events_map = NULL;
    system->local_components_map = NULL;
}

void system_clean(system_t *system) {
    if (NULL != system->local_events_map) {
        mem_free(system->local_events_map);
    }
    if (NULL != system->local_components_map) {
        mem_free(system->local_components_map);
    }
}

void system_init_local_events_map(system_t *system, size_t size) {
    if (NULL == system->local_events_map) {
        system->local_events_map = mem_alloc(sizeof(*(system->local_events_map)) * size);
    }
}

void system_init_local_components_map(system_t *system, size_t size) {
    if (NULL == system->local_components_map) {
        system->local_components_map = mem_alloc(sizeof(*(system->local_components_map)) * size);
    }
}
