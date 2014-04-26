#include "macros.h"
#include "system.h"
#include "mem_wrap.h"
#include <stdio.h>


system_t * system_new(void) {
    system_t *system = mem_alloc(sizeof(system_t));
    system->name = NOT_INITIALIZED;
    system->local_events_map = NULL;
    system->local_components_map = NULL;
    system->data = MAYBIFY(NULL);
    system->data_free = MAYBIFY_FUNC(NULL);
    return system;
}

void system_free(system_t *system) {
    if (NULL != system->local_events_map) {
        mem_free(system->local_events_map);
    }
    if (NULL != system->local_components_map) {
        mem_free(system->local_components_map);
    }
    if (NULL != UNMAYBE(system->data) && NULL != UNMAYBE(system->data_free)) {
        ((free_callback_t) UNMAYBE(system->data_free))(UNMAYBE(system->data));
    }
    mem_free(system);
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
