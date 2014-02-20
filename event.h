#ifndef __EVENT_H__
#define __EVENT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"


typedef struct event_s event_t;
typedef struct event_array_s event_array_t;
typedef struct events_map_s events_map_t;
typedef struct events_list_s events_list_t;


#include <stdlib.h>
#include "event_types.h"
#include "system.h"
#include "int_list.h"

#define CUSTOM_IDENTIFIER 0x10000000
#define DECLARE_EVENT(var) __CUSTOM_EVENT_ ## var
#define CUSTOM_EVENT(name) (DECLARE_EVENT(name) | CUSTOM_IDENTIFIER)
#define IMPORTED_EVENTS enum __IMPORTED_EVENTS {
#define END_IMPORTED_EVENTS ,__IMPORTED_EVENTS_COUNT};


#define EVENT_NAME_MAX_LENGTH 100
typedef void (*event_hook_t)(system_t *system, MAYBE(void *) params);

struct event_s {
    link_t events_link;
    event_type_t type;
    MAYBE(void *) params;
};

struct event_array_s {
    size_t count;
    event_t *events;
};

struct events_list_s {
    bool running;
    list_t events;
};

struct events_map_s {
    bool initialized;
    size_t count;
    list_t *hooks_map;
    list_t pending_imports;
    list_t pending_exports;
    list_t pending_hooks;
};

typedef struct registered_hook_s {
    link_t hooks_link;
    event_hook_t hook;
    system_t *system;
} registered_hook_t;

typedef struct pending_import_s {
    link_t pending_imports_link;
    system_t *system;
    const char *name;
    size_t local_index;
} pending_import_t;

typedef struct pending_export_s {
    link_t pending_exports_link;
    const char *name;
} pending_export_t;

typedef struct pending_hook_s {
    link_t pending_hooks_link;
    registered_hook_t *registered_hook;
    size_t event_id;
} pending_hook_t;

void events_map_init(events_map_t *events_map);

void events_map_export(events_map_t *events_map, const char *name); 

#define events_map_import(map, system, name) \
    do { \
        system_init_local_map(system, __IMPORTED_EVENTS_COUNT); \
        _events_map_import(map, system, #name, DECLARE_EVENT(name));\
    } while (0);

void _events_map_import(events_map_t *events_map, system_t *system, const char *name, size_t local_index);

void events_map_register_hook(events_map_t *events_map, system_t *system, event_hook_t hook, event_type_t event);

void events_map_process_pending(events_map_t *events_map);

void events_map_loop(events_map_t *events_map);

void events_map_clean(events_map_t *events_map);

#ifdef __cplusplus
}
#endif

#endif
