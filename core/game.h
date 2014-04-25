#ifndef __GAME_H__
#define __GAME_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"
#include "int_list.h"

typedef struct game_s game_t;
#include "component.h"
#include "entity.h"
#include "system.h"

typedef void (*event_hook_t)(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params);


#include "event.h"



struct game_s {
    bool paused;
    entities_list_t entities_list;
    events_map_t events_map;
    list_t events_queue;
    components_map_t components_map;
    list_t systems;
};

void game_init(game_t *game);

void game_clean(game_t *game);

void game_load_systems(game_t *game);

void game_start(game_t *game);

component_t * entity_add_component(game_t *game, system_t *system, entity_t *entity, uint32_t component_id);

entity_t * entity_create(game_t *game, char *name);

void game_register_hook(game_t *game, system_t *system, event_hook_t hook, MAYBE(void *) system_params, uint32_t event_id, MAYBE_FUNC(free_callback_t) system_params_free);

void game_push_event(game_t *game, system_t *system, uint32_t type, MAYBE(void *) sender_params);

void game_toggle_pause(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params);

#define game_export_event(game, system, name, sender_params_free) \
    do { \
        system_init_local_events_map(system, __LOCAL_EVENTS_COUNT); \
        _events_map_export(&((game)->events_map), #name, sender_params_free);\
        _events_map_import(&((game)->events_map), system, #name, name);\
    } while (0);

#define game_import_event(game, system, name) \
    do { \
        system_init_local_events_map(system, __LOCAL_EVENTS_COUNT); \
        _events_map_import(&((game)->events_map), system, #name, name);\
    } while (0);

#define game_export_component(game, system, name) \
    do { \
        system_init_local_components_map(system, __LOCAL_COMPONENTS_COUNT); \
        _components_map_export(&((game)->components_map), #name, INIT_DATA(name), CLEAN_DATA(name), sizeof(COMPONENT_DATA(name)));\
        _components_map_import(&((game)->components_map), system, #name, name);\
    } while (0);

#define game_import_component(game, system, name) \
    do { \
        system_init_local_components_map(system, __LOCAL_COMPONENTS_COUNT); \
        _components_map_import(&((game)->components_map), system, #name, name);\
    } while (0);

    
#ifdef __cplusplus
}
#endif

#endif
