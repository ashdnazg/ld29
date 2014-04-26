#include "game.h"

#include "macros.h"
#include "int_list.h"
#include "component.h"
#include "entity.h"
#include "system.h"
#include "event.h"
#include "tween.h"
#include "builtin_events.h"
#include <stdio.h>

void game_init(game_t *game) {
    game->paused = TRUE;
    entities_list_init(&(game->entities_list));
    components_map_init(&(game->components_map));
    list_init(&(game->events_queue), event_t, events_link);
    events_map_init(&(game->events_map));
    list_init(&(game->systems), system_t, systems_link);
    tween_list_init(&(game->tween_list));
}

void game_clean(game_t *game) {
    entities_list_clean(&(game->entities_list));
    components_map_clean(&(game->components_map));
    list_for_each(&(game->events_queue), event_t *, event) {
        event_free(&(game->events_map), event);
    }
    events_map_clean(&(game->events_map));
    list_init(&(game->systems), system_t, systems_link);
    tween_list_clean(&(game->tween_list));
}

void game_step(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    tween_list_tween(&(game->tween_list));
}


void game_load_systems(game_t *game) {
    game_register_hook(game, NULL, game_toggle_pause, MAYBIFY(NULL), EVENT_TOGGLE_PAUSE, MAYBIFY_FUNC(NULL));
    game_register_hook(game, NULL, game_step, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
}

void game_push_event(game_t *game, system_t *system, uint32_t type, MAYBE(void *) sender_params) {
    list_insert_tail(&(game->events_queue), event_new(GET_EVENT_ID(system, type), sender_params));
}

void game_start(game_t *game) {
    events_map_process_pending(&(game->events_map));
    components_map_process_pending(&(game->components_map));
    //events_map_loop(&(game->events_map));
    event_t * current_event;
    //events_queue.running = TRUE;
    game->paused = FALSE;
    game_push_event(game, NULL, EVENT_START, MAYBIFY(NULL));
    while (!list_is_empty(&(game->events_queue))) {
        current_event = (event_t *) list_head(&(game->events_queue));
        list_for_each(&(game->events_map.hooks_map[current_event->type]), registered_hook_t *, registered_hook) {
            registered_hook->hook(game, registered_hook->system, registered_hook->system_params, current_event->sender_params);
        }
        event_free(&(game->events_map), current_event);
    }
}

component_t * entity_add_component(game_t *game, system_t *system, entity_t *entity, uint32_t component_id) {
    component_t *component = component_new(&(game->components_map), GET_COMPONENT_ID(system, component_id));
    list_insert_tail(&(entity->components), component);
    return component;
}

entity_t * entity_create(game_t *game, char *name) {
    entity_t *entity = entity_new(name, game->entities_list.count);
    list_insert_tail(&(game->entities_list.entities), entity);
    game->entities_list.count += 1;
    
    return entity;
}

void game_register_hook(game_t *game, system_t *system, event_hook_t hook, MAYBE(void *) system_params, uint32_t event_id, MAYBE_FUNC(free_callback_t) system_params_free) {
    events_map_register_hook(&(game->events_map), system, hook, system_params, event_id, system_params_free);
}

void game_toggle_pause(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    if (game->paused) {
        game->paused = FALSE;
        game_push_event(game, NULL, EVENT_UNPAUSE, MAYBIFY(NULL));
        printf("unpaused\n");
    } else {
        game->paused = TRUE;
        game_push_event(game, NULL, EVENT_PAUSE, MAYBIFY(NULL));
        printf("paused\n");
    }
}
