#include "actors.h"

#include "core/mem_wrap.h"
#include "core/game.h"
#include "core/system.h"

#include "systems/sdl/sdl.h"
#include "systems/sdl/sdl_video.h"
#include "systems/map/map.h"

#include <stdint.h>

void actor_move_towards(actor_t *actor, int32_t dest_x, int32_t dest_y) {
    int delta;
    if (dest_x != actor->x) {
        delta = ABS(dest_x - actor->x) / (dest_x - actor->x);
        actor->x += delta;
        actor->renderable->x += delta;
    }
    if (dest_y != actor->y) {
        delta = ABS(dest_y - actor->y) / (dest_y - actor->y);
        actor->y += delta;
        actor->renderable->y += delta;
    }
}

void actors_update(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    actor_action_t actor_action;
    uint32_t dest_x, dest_y;
    sys_actors_data_t *sys_actors_data = (sys_actors_data_t *) UNMAYBE(system->data);
    
    list_for_each(&(sys_actors_data->actors), actor_t *, actor) {
        if (NULL != UNMAYBE(actor->ai.get_action)) {
            
            actor_action = ((ai_func_t) UNMAYBE(actor->ai.get_action))(actor, actor->ai.ai_params, &dest_x, &dest_y);
            
            switch(actor_action) {
                case ACTOR_ACTION_MOVE:
                    actor_move_towards(actor, dest_x, dest_y);
                case ACTOR_ACTION_IDLE:
                    break;
                default:
                    printf("bad actor_action: %d\n", actor_action);
                    exit(1);
            }
        }
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
