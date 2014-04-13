#include "game.h"

#include "macros.h"
#include "int_list.h"
#include "component.h"
#include "entity.h"
#include "system.h"
#include "event.h"


void game_init(game_t *game) {
    entities_list_init(&(game->entities_list));
    components_map_init(&(game->components_map));
    events_map_init(&(game->events_map));
    list_init(&(game->systems), system_t, systems_link);
}

void game_clean(game_t *game) {
    entities_list_clean(&(game->entities_list));
    components_map_clean(&(game->components_map));
    events_map_clean(&(game->events_map));
    list_init(&(game->systems), system_t, systems_link);
}

void game_load_systems(game_t *game) {
    
}

void game_start(game_t *game) {
    events_map_process_pending(&(game->events_map));
    components_map_process_pending(&(game->components_map));
    events_map_loop(&(game->events_map));
}

component_t * entity_add_component(game_t *game, entity_t *entity, uint32_t component_type_id) {
    component_t *component = component_new(&(game->components_map), component_type_id);
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
