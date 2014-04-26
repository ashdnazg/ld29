#include "actors.h"

#include "core/mem_wrap.h"
#include "core/game.h"
#include "core/system.h"

#include "systems/sdl/sdl.h"
#include "systems/sdl/sdl_video.h"
#include "systems/map/map.h"

#include <stdint.h>

void actor_move_towards(actor_t *actor, int32_t dest_x, int32_t dest_y) {
    int x_delta = 0, y_delta = 0;
    if (dest_x != actor->x) {
        x_delta = ABS(dest_x - actor->x) / (dest_x - actor->x);
        if (map_rect_passable(actor->map, actor->x + x_delta, actor->y, ACTOR_SIZE, ACTOR_SIZE)) {
            actor->x += x_delta;
        }
    }
    if (dest_y != actor->y) {
        y_delta = ABS(dest_y - actor->y) / (dest_y - actor->y);
        if (map_rect_passable(actor->map, actor->x, actor->y + y_delta, ACTOR_SIZE, ACTOR_SIZE)) {
            actor->y += y_delta;
        }
    }
}

void actor_move_dijkstra(actor_t * actor, dijkstra_map_t *d_map) {
    int i,j;
    uint32_t actor_tile_x, actor_tile_y, dest_tile_x, dest_tile_y;
    int32_t dest_x, dest_y;
    uint32_t minimal_value = UINT32_MAX;
    int x_delta = 0, y_delta = 0;
    map_translate_coordinates(actor->map, actor->x, actor->y, &actor_tile_x, &actor_tile_y);
    
    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            // if (i == 0 && j == 0) {
                // continue;
            // }
            if (!map_in_map(actor->map, actor_tile_x + j, actor_tile_y + i)) {
                continue;
            }
            if (minimal_value > d_map->matrix[COORD(d_map, actor_tile_x + j, actor_tile_y + i)]) {
                minimal_value = d_map->matrix[COORD(d_map, actor_tile_x + j, actor_tile_y + i)];
                x_delta = j;
                y_delta = i;
            }
        }
    }
    dest_tile_x = actor_tile_x + x_delta;
    dest_tile_y = actor_tile_y + y_delta;
    if (!map_rect_passable(actor->map, actor->x + x_delta, actor->y, ACTOR_SIZE, ACTOR_SIZE)) {
        x_delta = 0;
    }
    if (!map_rect_passable(actor->map, actor->x, actor->y + y_delta, ACTOR_SIZE, ACTOR_SIZE)) {
        y_delta = 0;
    }
    if (x_delta == 0 && y_delta == 0) {
        map_tile_center(actor->map, dest_tile_x, dest_tile_y, &dest_x, &dest_y);
        actor_move_towards(actor, dest_x - ACTOR_SIZE / 2, dest_y - ACTOR_SIZE / 2);
    } else {
        actor->x += x_delta;
        actor->y += y_delta;
    }
}

void actors_update(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    actor_action_t actor_action;
    uint32_t dest_tile_x, dest_tile_y;
    dijkstra_map_t *d_map;
    sys_actors_data_t *sys_actors_data = (sys_actors_data_t *) UNMAYBE(system->data);
    
    list_for_each(&(sys_actors_data->actors), actor_t *, actor) {
        if (NULL != UNMAYBE(actor->ai.get_action)) {
            
            actor_action = ((ai_func_t) UNMAYBE(actor->ai.get_action))(actor, actor->ai.ai_params, &dest_tile_x, &dest_tile_y);
            
            switch(actor_action) {
                case ACTOR_ACTION_MOVE:
                    d_map = map_create_dijkstra(actor->map, dest_tile_x, dest_tile_y);
                    actor_move_dijkstra(actor, d_map);
                    dijkstra_map_free(d_map);
                case ACTOR_ACTION_IDLE:
                    break;
                default:
                    printf("bad actor_action: %d\n", actor_action);
                    exit(1);
            }
        }
        actor->renderable->x = actor->x;
        actor->renderable->y = actor->y;
    }
}


void actor_free(actor_t *actor) {
    link_remove_from_list(&(actor->actors_link));
    //renderable_free(actor->renderable);
    mem_free(actor);
}

actor_t * sys_actors_add_actor(game_t *game, map_t *map, actor_type_t type, int32_t x, int32_t y) {
    system_t *sys_actors = game_get_system(game, SYS_ACTORS_NAME);
    sys_actors_data_t *sys_actors_data = (sys_actors_data_t *) UNMAYBE(sys_actors->data);
    actor_t *actor = mem_alloc(sizeof(*actor));
    const char *type_name = NULL;
    link_init(&(actor->actors_link));
    actor->map = map;
    actor->type = type;
    actor->ai.get_action = MAYBIFY_FUNC(NULL);
    actor->ai.ai_params = MAYBIFY(NULL);
    actor->x = x;
    actor->y = y;
    
    switch (type) {
        case ACTOR_CAPTAIN:
            type_name = "captain";
            break;
        case ACTOR_OFFICER:
            type_name = "officer";
            break;
        case ACTOR_SOLDIER:
            type_name = "soldier";
            break;
        default:
            printf("wrong actor type: %d", type);
            exit(1);
    }
    actor->renderable = sys_SDL_add_renderable(game, type_name, x, y, ACTOR_DEPTH);
    
    
    list_insert_tail(&(sys_actors_data->actors), actor);
    return actor;
    
}

void actors_clean(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_actors_data_t *sys_actors_data = (sys_actors_data_t *) UNMAYBE(system->data);
    list_for_each(&(sys_actors_data->actors), actor_t *, actor) {
        actor_free(actor);
    }
}


bool actors_start(game_t *game, system_t *system) {
    system->name = SYS_ACTORS_NAME;
    sys_actors_data_t *sys_actors_data = mem_alloc(sizeof(*sys_actors_data));
    list_init(&(sys_actors_data->actors), actor_t, actors_link);
    
    system->data = MAYBIFY(sys_actors_data);
    system->data_free = MAYBIFY_FUNC(mem_free);
    game_register_hook(game, system, actors_clean, MAYBIFY(NULL), EVENT_EXIT, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, actors_update, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    
    return TRUE;
}
