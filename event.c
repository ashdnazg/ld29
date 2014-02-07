#include "event.h"
#include "system.h"
#include "mem_wrap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

inline bool event_is_custom(size_t event) {
    return (event & CUSTOM_IDENTIFIER);
}

inline size_t get_custom_event_local_id(size_t event) {
    return (event ^ CUSTOM_IDENTIFIER);
}

void events_map_init(events_map_t *events_map) {
    events_map->initialized = FALSE;
    events_map->count = EVENT_COUNT;
    events_map->hooks_map = NULL;
    list_init(&(events_map->pending_imports), pending_import_t, pending_imports_link);
    list_init(&(events_map->pending_exports), pending_export_t, pending_exports_link);
    list_init(&(events_map->pending_hooks), pending_hook_t, pending_hooks_link);
}



void events_map_export(events_map_t *events_map, const char *name) {
    pending_export_t *pending_export = mem_alloc(sizeof(*pending_export));
    link_init(&(pending_export->pending_exports_link));
    pending_export->name = name;
    
    list_insert_tail(&(events_map->pending_exports), pending_export);
    events_map->count += 1;
}


void _events_map_import(events_map_t *events_map, system_t *system, const char *name, size_t local_index) {
    pending_import_t *pending_import = mem_alloc(sizeof(*pending_import));
    link_init(&(pending_import->pending_imports_link));
    pending_import->name = name;
    pending_import->system = system;
    pending_import->local_index = local_index;
    
    list_insert_tail(&(events_map->pending_imports), pending_import);
}

registered_hook_t * registered_hook_new(system_t *system, event_hook_t hook) {
    registered_hook_t *registered_hook = mem_alloc(sizeof(*registered_hook));
    link_init(&(registered_hook->hooks_link));
    registered_hook->system = system;
    registered_hook->hook = hook;
    
    return registered_hook;
}

void registered_hook_free(registered_hook_t *registered_hook) {
    link_remove_from_list(&(registered_hook->hooks_link));
    mem_free(registered_hook);
}

void events_map_register_hook(events_map_t *events_map, system_t *system, event_hook_t hook, size_t event_id) {
    pending_hook_t *pending_hook = mem_alloc(sizeof(*pending_hook));
    link_init(&(pending_hook->pending_hooks_link));
    pending_hook->registered_hook = registered_hook_new(system, hook);
    pending_hook->event_id = event_id;
    
    list_insert_tail(&(events_map->pending_hooks), pending_hook);
}

bool compare_pending_exports(pending_export_t *pending_export_a, pending_export_t *pending_export_b) {
    return strncmp(pending_export_a->name, pending_export_b->name, EVENT_NAME_MAX_LENGTH) < 0;
}

bool compare_pending_imports(pending_import_t *pending_import_a, pending_import_t *pending_import_b) {
    return strncmp(pending_import_a->name, pending_import_b->name, EVENT_NAME_MAX_LENGTH) < 0;
}

void pending_import_free(pending_import_t *pending_import) {
    link_remove_from_list(&(pending_import->pending_imports_link));
    mem_free(pending_import);
}

void pending_export_free(pending_export_t *pending_export) {
    link_remove_from_list(&(pending_export->pending_exports_link));
    mem_free(pending_export);
}

void pending_hook_free(pending_hook_t *pending_hook) {
    link_remove_from_list(&(pending_hook->pending_hooks_link));
    if (!link_is_linked(&(pending_hook->registered_hook->hooks_link))) {
        registered_hook_free(pending_hook->registered_hook);
    }
    mem_free(pending_hook);
}

void events_map_process_pending(events_map_t *events_map) {
    int i;
    int cmp;
    int map_index;
    list_t *list_ptr = NULL;
    events_map->hooks_map = mem_alloc(sizeof(*events_map->hooks_map) * events_map->count);
    
    list_ptr = events_map->hooks_map;
    for (i = 0; i < events_map->count; ++i) {
        list_init(list_ptr, registered_hook_t, hooks_link);
        ++list_ptr;
    }
    
    list_sort(&(events_map->pending_imports), (cmp_cb_t) compare_pending_imports);
    list_sort(&(events_map->pending_exports), (cmp_cb_t) compare_pending_exports);
    i = EVENT_COUNT;
    list_for_each(&(events_map->pending_exports), pending_export_t *, pending_export) {
        if (next_pending_export != NULL && compare_pending_exports(next_pending_export, pending_export)) {
            //printf("Two events with the name: %s\n", pending_export->name);
            //ERROR: two events with the same name
        }
        list_for_each(&(events_map->pending_imports), pending_import_t *, pending_import) {
            cmp = strncmp(pending_import->name, pending_export->name, EVENT_NAME_MAX_LENGTH);
            if (cmp == 0) {
                pending_import->system->local_events_map[pending_import->local_index] = i;
                //printf("Imported: %s mapped: %d -> %d\n", pending_import->name, pending_import->local_index, i);
                pending_import_free(pending_import);
            } else if (cmp < 0) {
                printf("Imported event not found: %s\n", pending_import->name);
                //ERROR: couldn't import an event
            }
            
        }
        pending_export_free(pending_export);
        ++i;
    }
    list_for_each(&(events_map->pending_hooks), pending_hook_t *, pending_hook) {
        if (event_is_custom(pending_hook->event_id)) {
            map_index = pending_hook->registered_hook->system->local_events_map[get_custom_event_local_id(pending_hook->event_id)];
        } else {
            map_index = pending_hook->event_id;
        }
        list_insert_tail(&(events_map->hooks_map[map_index]), pending_hook->registered_hook);
        //printf("Registered hook for event: %d\n", map_index);
        pending_hook_free(pending_hook);
    }
    events_map->initialized = TRUE;
}

void events_map_clean(events_map_t *events_map) {
    int i = 0;
    list_t *list_ptr = NULL;
    
    if (NULL != events_map->hooks_map) {
        list_ptr = events_map->hooks_map;
        for (i = 0; i < events_map->count; ++i) {
            list_for_each(&(events_map->hooks_map[i]), registered_hook_t *, registered_hook) {
                registered_hook_free(registered_hook);
            }
            ++list_ptr;
        }
        mem_free(events_map->hooks_map);
    }
    list_for_each(&(events_map->pending_imports), pending_import_t *, pending_import) {
        pending_import_free(pending_import);
    }
    list_for_each(&(events_map->pending_exports), pending_export_t *, pending_export) {
        pending_export_free(pending_export);
    }
    list_for_each(&(events_map->pending_hooks), pending_hook_t *, pending_hook) {
        pending_hook_free(pending_hook);
    }
}
