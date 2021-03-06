#ifndef __ACTORS_H__
#define __ACTORS_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "core/macros.h"
#include "core/system.h"
#include "core/game.h"
#include "core/int_list.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include "systems/sdl/sdl_video.h"
#include "systems/map/map.h"


#define ACTOR_MAX_SPEED 9
#define ACTOR_DEPTH 10
#define ACTOR_SIZE 5
#define SYS_ACTORS_NAME "actors"

#define FINE_FACTOR 3

typedef enum actor_action_e {
    ACTOR_ACTION_IDLE,
    ACTOR_ACTION_WORK,
    ACTOR_ACTION_MOVE
} actor_action_t;

typedef enum actor_type_e {
    ACTOR_CAPTAIN,
    ACTOR_OFFICER,
    ACTOR_SOLDIER
} actor_type_t;

typedef struct actor_s actor_t;

typedef actor_action_t (*ai_func_t)(game_t *game, actor_t *actor, MAYBE(void *) ai_params, uint32_t *out_x, uint32_t *out_y);

typedef struct actor_ai_s {
    MAYBE_FUNC(ai_func_t) get_action;
    MAYBE(void *) ai_params;
    MAYBE_FUNC(free_callback_t) ai_params_free;
} actor_ai_t;

struct actor_s {
    bool alive;
    link_t actors_link;
    map_t *map;
    actor_type_t type;
    actor_ai_t ai;
    renderable_t *renderable;
    int32_t x;
    int32_t fine_x;
    int32_t y;
    int32_t fine_y;
    uint32_t speed;
};



typedef struct sys_actors_data_s {
    list_t actors;
} sys_actors_data_t;


void actor_kill(game_t *game, actor_t *actor);

bool actors_in_same_room(actor_t *a, actor_t *b);

actor_t * sys_actors_add_actor(game_t *game, map_t *map, actor_type_t type, int32_t x, int32_t y);
bool actors_start(game_t *game, system_t *system);


#ifdef __cplusplus
}
#endif

#endif
