#include "logic.h"

#include "core/mem_wrap.h"
#include "core/game.h"
#include "core/system.h"

#include "systems/sdl/sdl.h"
#include "systems/actors/actors.h"
#include "systems/map/map.h"
#include <stdint.h>

LOCAL_EVENTS
    sdl_left_mouse_down,
    sdl_right_mouse_down
END_LOCAL_EVENTS

void update_controller(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_mouse_down_data_t *mouse_data = UNMAYBE(sender_params);
    game_state_t * game_state = UNMAYBE(system_params);
    game_state->controller_x = mouse_data->x;
    game_state->controller_y = mouse_data->y;
}

actor_action_t get_controller_action(actor_t *actor, MAYBE(void *) ai_params, uint32_t *out_x, uint32_t *out_y) {
    game_state_t * game_state = UNMAYBE(ai_params);
    if (game_state->controller_x == actor->x && game_state->controller_y == actor->y) {
        game_state->controller_x = DESTINATION_NOT_SET;
        game_state->controller_y = DESTINATION_NOT_SET;
        return ACTOR_ACTION_IDLE;
    }
    if (game_state->controller_x != DESTINATION_NOT_SET && game_state->controller_y != DESTINATION_NOT_SET) {
        *out_x = game_state->controller_x;
        *out_y = game_state->controller_y;
        return ACTOR_ACTION_MOVE;
    }
    return ACTOR_ACTION_IDLE;
}

void logic_startup(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    game_state_t * game_state = UNMAYBE(system_params);
    game_state->current_map = map_get_main_map(game);
    actor_t *captain = sys_actors_add_actor(game, game_state->current_map, ACTOR_CAPTAIN, 200, 70);
    captain->ai.get_action = MAYBIFY_FUNC(get_controller_action);
    captain->ai.ai_params = MAYBIFY(game_state);
}

bool logic_start(game_t *game, system_t *system) {
    system->name = SYS_LOGIC_NAME;
    game_state_t * game_state = mem_alloc(sizeof(*game_state));
    game_state->controller_x = DESTINATION_NOT_SET;
    game_state->controller_y = DESTINATION_NOT_SET;
    
    system->data = MAYBIFY(game_state);
    system->data_free = MAYBIFY_FUNC(mem_free);
    
    game_import_event(game, system, sdl_left_mouse_down);
    game_import_event(game, system, sdl_right_mouse_down);
    game_register_hook(game, system, logic_startup, MAYBIFY(game_state), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, update_controller, MAYBIFY(game_state), sdl_right_mouse_down, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, update_controller, MAYBIFY(game_state), sdl_left_mouse_down, MAYBIFY_FUNC(NULL));
    
    return TRUE;
}
