#ifndef __EVENT_H__
#define __EVENT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"


typedef struct event_s event_t;
typedef struct events_map_s events_map_t;
typedef struct events_queue_s events_queue_t;

#include <stdlib.h>
#include <stdint.h>
#include "builtin_events.h"
#include "system.h"
#include "int_list.h"

struct event_s {
    link_t events_link;
    uint32_t type;
    MAYBE(void *) sender_params;
};

struct events_map_s {
    bool initialized;
    size_t count;
    list_t *hooks_map;
    MAYBE_FUNC(free_callback_t) *sender_params_free;
    list_t pending_event_imports;
    list_t pending_event_exports;
    list_t pending_hooks;
};



#include "game.h"

#define EVENT_IS_CUSTOM(event_id) (event_id & 0x40000000)
#define GET_CUSTOM_EVENT_ID(system, custom_id) ((system)->local_events_map[(custom_id) & 0x3FFFFFFF])
#define GET_EVENT_ID(system, event_id) (EVENT_IS_CUSTOM(event_id) ? GET_CUSTOM_EVENT_ID(system, event_id) : event_id)
#define LOCAL_EVENTS enum __LOCAL_EVENTS { __DUMMY_EVENT = 0x3FFFFFFF,
#define END_LOCAL_EVENTS ,__DUMMY_LAST_EVENT};
#define __LOCAL_EVENTS_COUNT (__DUMMY_LAST_EVENT & 0x3FFFFFFF)

#define EVENT_NAME_MAX_LENGTH 100



typedef struct registered_hook_s {
    link_t hooks_link;
    event_hook_t hook;
    system_t *system;
    MAYBE(void *) system_params;
    MAYBE_FUNC(free_callback_t) system_params_free;
} registered_hook_t;

typedef struct pending_event_import_s {
    link_t pending_event_imports_link;
    system_t *system;
    const char *name;
    uint32_t local_index;
} pending_event_import_t;

typedef struct pending_event_export_s {
    link_t pending_event_exports_link;
    const char *name;
    MAYBE_FUNC(free_callback_t) sender_params_free;
} pending_event_export_t;

typedef struct pending_hook_s {
    link_t pending_hooks_link;
    registered_hook_t *registered_hook;
    uint32_t event_id;
} pending_hook_t;


event_t * event_new(uint32_t type, MAYBE(void *) sender_params);

void event_free(events_map_t *events_map, event_t * event);

void events_map_init(events_map_t *events_map);

#define events_map_export(map, system, name, sender_params_free) \
    do { \
        system_init_local_events_map(system, __LOCAL_EVENTS_COUNT); \
        _events_map_export(map, #name, sender_params_free);\
        _events_map_import(map, system, #name, CUSTOM_EVENT(name));\
    } while (0);

void _events_map_export(events_map_t *events_map, const char *name, MAYBE_FUNC(free_callback_t) sender_params_free);

#define events_map_import(map, system, name) \
    do { \
        system_init_local_events_map(system, __LOCAL_EVENTS_COUNT); \
        _events_map_import(map, system, #name, CUSTOM_EVENT(name));\
    } while (0);

void _events_map_import(events_map_t *events_map, system_t *system, const char *name, uint32_t local_index);

void events_map_register_hook(events_map_t *events_map, system_t *system, event_hook_t hook, MAYBE(void *) system_params, uint32_t event_id, MAYBE_FUNC(free_callback_t) system_params_free);

void events_map_process_pending(events_map_t *events_map);

void events_map_clean(events_map_t *events_map);

#ifdef __cplusplus
}
#endif

#endif
