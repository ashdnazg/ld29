#include "event.h"
#include "system.h"
#include "mem_wrap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


void events_map_init(events_map_t *events_map) {
    events_map->initialized = FALSE;
    events_map->count = EVENTS_COUNT;
    events_map->hooks_map = NULL;
    events_map->sender_params_free = NULL;
    list_init(&(events_map->pending_event_imports), pending_event_import_t, pending_event_imports_link);
    list_init(&(events_map->pending_event_exports), pending_event_export_t, pending_event_exports_link);
    list_init(&(events_map->pending_hooks), pending_hook_t, pending_hooks_link);
}



void _events_map_export(events_map_t *events_map, const char *name, MAYBE_FUNC(free_callback_t) sender_params_free) {
    pending_event_export_t *pending_event_export = mem_alloc(sizeof(*pending_event_export));
    link_init(&(pending_event_export->pending_event_exports_link));
    pending_event_export->name = name;
    pending_event_export->sender_params_free = sender_params_free;
    list_insert_tail(&(events_map->pending_event_exports), pending_event_export);
    ++(events_map->count);
}


void _events_map_import(events_map_t *events_map, system_t *system, const char *name, uint32_t local_index) {
    pending_event_import_t *pending_event_import = mem_alloc(sizeof(*pending_event_import));
    link_init(&(pending_event_import->pending_event_imports_link));
    pending_event_import->name = name;
    pending_event_import->system = system;
    pending_event_import->local_index = local_index;
    
    list_insert_tail(&(events_map->pending_event_imports), pending_event_import);
}

event_t * event_new(uint32_t type, MAYBE(void *) sender_params) {
    event_t *event = mem_alloc(sizeof(*event));
    link_init(&(event->events_link));
    event->type = type;
    event->sender_params = sender_params;
    
    return event;
}

void event_free(events_map_t *events_map, event_t * event) {
    link_remove_from_list(&(event->events_link));
    if (UNMAYBE(event->sender_params) != NULL && UNMAYBE(events_map->sender_params_free[event->type]) != NULL) {
        ((free_callback_t) UNMAYBE(events_map->sender_params_free[event->type]))(UNMAYBE(event->sender_params));
    }
    mem_free(event);
}


void push_event(events_queue_t *events_queue, uint32_t type, MAYBE(void *) sender_params) {
    list_insert_tail(&(events_queue->events), event_new(type, sender_params));
}

void events_map_loop(events_map_t *events_map) {
    events_queue_t events_queue;
    event_t * current_event;
    list_init(&(events_queue.events), event_t, events_link);
    events_queue.running = TRUE;
    
    while (events_queue.running && !list_is_empty(&(events_queue.events))) {
        current_event = (event_t *) list_head(&(events_queue.events));
        list_for_each(&(events_map->hooks_map[(uint32_t) (current_event->type)]), registered_hook_t *, registered_hook) {
            registered_hook->hook(&events_queue, registered_hook->system, registered_hook->system_params, current_event->sender_params);
        }
        event_free(events_map, current_event);
    }
    list_for_each(&(events_queue.events), event_t *, event) {
        event_free(events_map, event);
    }
}



registered_hook_t * registered_hook_new(event_hook_t hook, system_t *system, MAYBE(void *) system_params, MAYBE_FUNC(free_callback_t) system_params_free) {
    registered_hook_t *registered_hook = mem_alloc(sizeof(*registered_hook));
    link_init(&(registered_hook->hooks_link));
    registered_hook->hook = hook;
    registered_hook->system = system;
    registered_hook->system_params = system_params;
    registered_hook->system_params_free = system_params_free;
    
    return registered_hook;
}

void registered_hook_free(registered_hook_t *registered_hook) {
    link_remove_from_list(&(registered_hook->hooks_link));
    if (UNMAYBE(registered_hook->system_params) != NULL && UNMAYBE(registered_hook->system_params_free) != NULL) {
        ((free_callback_t) UNMAYBE(registered_hook->system_params_free))(UNMAYBE(registered_hook->system_params));
    }
    mem_free(registered_hook);
}

void events_map_register_hook(events_map_t *events_map, system_t *system, event_hook_t hook, MAYBE(void *) system_params, uint32_t event_id, MAYBE_FUNC(free_callback_t) system_params_free) {
    pending_hook_t *pending_hook = mem_alloc(sizeof(*pending_hook));
    link_init(&(pending_hook->pending_hooks_link));
    pending_hook->registered_hook = registered_hook_new(hook, system, system_params, system_params_free);
    pending_hook->event_id = event_id;
    
    list_insert_tail(&(events_map->pending_hooks), pending_hook);
}

bool compare_pending_event_exports(pending_event_export_t *pending_event_export_a, pending_event_export_t *pending_event_export_b) {
    return strncmp(pending_event_export_a->name, pending_event_export_b->name, EVENT_NAME_MAX_LENGTH) < 0;
}

bool compare_pending_event_imports(pending_event_import_t *pending_event_import_a, pending_event_import_t *pending_event_import_b) {
    return strncmp(pending_event_import_a->name, pending_event_import_b->name, EVENT_NAME_MAX_LENGTH) < 0;
}

void pending_event_import_free(pending_event_import_t *pending_event_import) {
    link_remove_from_list(&(pending_event_import->pending_event_imports_link));
    mem_free(pending_event_import);
}

void pending_event_export_free(pending_event_export_t *pending_event_export) {
    link_remove_from_list(&(pending_event_export->pending_event_exports_link));
    mem_free(pending_event_export);
}

void pending_hook_free(pending_hook_t *pending_hook) {
    link_remove_from_list(&(pending_hook->pending_hooks_link));
    if (!link_is_linked(&(pending_hook->registered_hook->hooks_link))) {
        registered_hook_free(pending_hook->registered_hook);
    }
    mem_free(pending_hook);
}

void init_builtin_events(events_map_t *events_map) {
    events_map->sender_params_free[EVENT_LOG] = MAYBIFY_FUNC(mem_free);
}

void events_map_process_pending(events_map_t *events_map) {
    int i;
    int cmp;
    list_t *list_ptr = NULL;
    events_map->hooks_map = mem_alloc(sizeof(*(events_map->hooks_map)) * events_map->count);
    events_map->sender_params_free = mem_alloc(sizeof(*(events_map->sender_params_free)) * events_map->count);
    
    for (i = 0; i < events_map->count; ++i) {
        events_map->sender_params_free[i] = MAYBIFY_FUNC(NULL);
    }
    init_builtin_events(events_map);
    
    list_ptr = events_map->hooks_map;
    for (i = 0; i < events_map->count; ++i) {
        list_init(list_ptr, registered_hook_t, hooks_link);
        ++list_ptr;
    }
    
    list_sort(&(events_map->pending_event_imports), (cmp_cb_t) compare_pending_event_imports);
    list_sort(&(events_map->pending_event_exports), (cmp_cb_t) compare_pending_event_exports);
    i = EVENTS_COUNT;
    list_for_each(&(events_map->pending_event_exports), pending_event_export_t *, pending_event_export) {
        if (next_pending_event_export != NULL && compare_pending_event_exports(next_pending_event_export, pending_event_export)) {
            printf("Two events with the name: %s\n", pending_event_export->name);
            //ERROR: two events with the same name
        }
        list_for_each(&(events_map->pending_event_imports), pending_event_import_t *, pending_event_import) {
            cmp = strncmp(pending_event_import->name, pending_event_export->name, EVENT_NAME_MAX_LENGTH);
            if (cmp == 0) {
                pending_event_import->system->local_events_map[pending_event_import->local_index] = i;
                //printf("Imported: %s mapped: %d -> %d\n", pending_event_import->name, pending_event_import->local_index, i);
                pending_event_import_free(pending_event_import);
            } else if (cmp < 0) {
                printf("Imported event not found: %s\n", pending_event_import->name);
                //ERROR: couldn't import an event
            }
            
        }
        events_map->sender_params_free[i] = pending_event_export->sender_params_free;
        pending_event_export_free(pending_event_export);
        ++i;
    }
    list_for_each(&(events_map->pending_hooks), pending_hook_t *, pending_hook) {
        list_insert_tail(&(events_map->hooks_map[pending_hook->event_id]), pending_hook->registered_hook);
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
    mem_free(events_map->sender_params_free);
    list_for_each(&(events_map->pending_event_imports), pending_event_import_t *, pending_event_import) {
        pending_event_import_free(pending_event_import);
    }
    list_for_each(&(events_map->pending_event_exports), pending_event_export_t *, pending_event_export) {
        pending_event_export_free(pending_event_export);
    }
    list_for_each(&(events_map->pending_hooks), pending_hook_t *, pending_hook) {
        pending_hook_free(pending_hook);
    }
}
