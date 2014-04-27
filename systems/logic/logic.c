#include "logic.h"

#include "core/mem_wrap.h"
#include "core/game.h"
#include "core/system.h"

#include "systems/sdl/sdl.h"
#include "systems/actors/actors.h"
#include "systems/map/map.h"
#include "systems/text/text.h"

#include <stdint.h>
#include <stdlib.h>

LOCAL_EVENTS
    sdl_left_mouse_down,
    sdl_right_mouse_down,
    commands_send_command
END_LOCAL_EVENTS

void update_controller(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    bool in_map;
    uint32_t map_x, map_y;
    sys_sdl_mouse_down_data_t *mouse_data = UNMAYBE(sender_params);
    game_state_t * game_state = UNMAYBE(system_params);
    in_map = map_translate_coordinates(game_state->current_map, mouse_data->x, mouse_data->y, &map_x, &map_y);
    if (in_map && map_tile_passable(game_state->current_map, map_x, map_y)) {
        game_state->controller_x = map_x;
        game_state->controller_y = map_y;
    }
}

actor_action_t get_controller_action(game_t *game, actor_t *actor, MAYBE(void *) ai_params, uint32_t *out_x, uint32_t *out_y) {
    int32_t dest_center_x, dest_center_y;
    uint32_t actor_tile_x, actor_tile_y;
    game_state_t * game_state = UNMAYBE(ai_params);
    map_translate_coordinates(game_state->current_map, actor->x, actor->y, &actor_tile_x, &actor_tile_y);
    if (game_state->controller_x != DESTINATION_NOT_SET && game_state->controller_y != DESTINATION_NOT_SET) {
        map_tile_center(game_state->current_map, game_state->controller_x, game_state->controller_y, &dest_center_x, &dest_center_y);
        if ((ABS(actor->x - dest_center_x + ACTOR_SIZE / 2) < 2 && ABS(actor->y - dest_center_y + ACTOR_SIZE / 2) < 2) || 
            (!map_reachable(actor->map, game_state->controller_x, game_state->controller_y, actor_tile_x, actor_tile_y))) {
            printf("stopped\n");
            game_state->controller_x = DESTINATION_NOT_SET;
            game_state->controller_y = DESTINATION_NOT_SET;
            return ACTOR_ACTION_IDLE;
        } else {
            *out_x = game_state->controller_x;
            *out_y = game_state->controller_y;
            return ACTOR_ACTION_MOVE;
        }
    }
    return ACTOR_ACTION_IDLE;
}

tile_type_t get_task_tile_type(task_t task) {
    switch(task) {
        case TASK_IDLE:
            return TILE_ANYWHERE;
        case TASK_EAT:
            return TILE_HOLD;
        case TASK_SLEEP:
            return TILE_CREW_QUARTERS;
        case TASK_LISTEN_SONAR:
            return TILE_SONAR;
        case TASK_STEER:
        case TASK_SET_COURSE:
        case TASK_WATCH_PERISCOPE:
            return TILE_CONTROL;
        case TASK_STOP_ENGINE:
        case TASK_START_ENGINE:
        case TASK_MONITOR_ENGINE:
            return TILE_ENGINES;
        case TASK_MONITOR_WEAPONS:
        case TASK_FIRE_TORPEDO:
            return TILE_WEAPONS;
        default: 
            printf("tried to get tile for task: %d", task);
            exit(1);
    }
}

actor_action_t soldier_switch_task(actor_t *actor, soldier_ai_params_t *soldier_ai_params, task_t new_task) {
    uint32_t actor_tile_x, actor_tile_y;
    soldier_ai_params->task = new_task;
    map_translate_coordinates(actor->map, actor->x, actor->y, &actor_tile_x, &actor_tile_y);
    
    soldier_ai_params->time_left = TASK_MIN_TIME + rand() % (TASK_MAX_TIME - TASK_MIN_TIME);
    tile_type_t required_tile = get_task_tile_type(new_task);
    if (required_tile == TILE_ANYWHERE || actor->map->matrix[COORD(actor->map, actor_tile_x, actor_tile_y)] == required_tile) {
        soldier_ai_params->destination_x = DESTINATION_NOT_SET;
        soldier_ai_params->destination_y = DESTINATION_NOT_SET;
        return ACTOR_ACTION_WORK;
    }
    map_get_random_tile(actor->map, required_tile, &(soldier_ai_params->destination_x), &(soldier_ai_params->destination_y));
    return ACTOR_ACTION_MOVE;
}

actor_action_t get_soldier_action(game_t *game, actor_t *actor, MAYBE(void *) ai_params, uint32_t *out_x, uint32_t *out_y) {
    soldier_ai_params_t *soldier_ai_params = UNMAYBE(ai_params);
    int32_t dest_center_x, dest_center_y;
    uint32_t actor_tile_x, actor_tile_y;
    actor_action_t action;
    
    if (actors_in_same_room(actor, soldier_ai_params->game_state->captain) && soldier_ai_params->game_state->pending_task != TASK_NO_TASK &&
        (!(soldier_ai_params->busy) || soldier_ai_params->task < soldier_ai_params->game_state->pending_task)) {
        action = soldier_switch_task(actor, soldier_ai_params, soldier_ai_params->game_state->pending_task);
        soldier_ai_params->game_state->pending_task = TASK_NO_TASK;
        *out_x = soldier_ai_params->destination_x;
        *out_y = soldier_ai_params->destination_y;
        soldier_ai_params->busy = TRUE;
        return action;
    }
    
    map_translate_coordinates(actor->map, actor->x, actor->y, &actor_tile_x, &actor_tile_y);
    if (soldier_ai_params->destination_x != DESTINATION_NOT_SET && soldier_ai_params->destination_y != DESTINATION_NOT_SET) {
        map_tile_center(actor->map, soldier_ai_params->destination_x, soldier_ai_params->destination_y, &dest_center_x, &dest_center_y);
        if ((ABS(actor->x - dest_center_x + ACTOR_SIZE / 2) < 2 && ABS(actor->y - dest_center_y + ACTOR_SIZE / 2) < 2) || 
            (!map_reachable(actor->map, soldier_ai_params->destination_x, soldier_ai_params->destination_y, actor_tile_x, actor_tile_y))) {
            soldier_ai_params->destination_x = DESTINATION_NOT_SET;
            soldier_ai_params->destination_y = DESTINATION_NOT_SET;
        } else {
            *out_x = soldier_ai_params->destination_x;
            *out_y = soldier_ai_params->destination_y;
            return ACTOR_ACTION_MOVE;
        }
    }
    
    if (soldier_ai_params->time_left > 0) {
        soldier_ai_params->time_left -= 1;
        switch (soldier_ai_params->task) {
            case TASK_LISTEN_SONAR:
                soldier_ai_params->game_state->sonar_listened = TRUE;
                break;
            case TASK_WATCH_PERISCOPE:
                soldier_ai_params->game_state->periscope_watched = TRUE;
                break;
            case TASK_MONITOR_ENGINE:
                soldier_ai_params->game_state->engine_monitored = TRUE;
                break;
            case TASK_MONITOR_WEAPONS:
                soldier_ai_params->game_state->weapons_monitored = TRUE;
                break;
            case TASK_SET_COURSE:
                soldier_ai_params->game_state->setting_course = TRUE;
                break;
            case TASK_STOP_ENGINE:
                soldier_ai_params->game_state->stopping_engine = TRUE;
                break;
            case TASK_START_ENGINE:
                soldier_ai_params->game_state->starting_engine = TRUE;
                break;
            case TASK_STEER:
                soldier_ai_params->game_state->steering = TRUE;
                break;
            case TASK_FIRE_TORPEDO:
                soldier_ai_params->game_state->firing = TRUE;
                break;
            case TASK_SEAL_LEAK:
                soldier_ai_params->game_state->sealing_leak = TRUE;
                break;
            default:
                break;
        }
        return ACTOR_ACTION_WORK;
    }
    if (soldier_ai_params->busy == TRUE) {
        switch (soldier_ai_params->task) {
            case TASK_SET_COURSE:
                if (soldier_ai_params->game_state->moving) {
                    soldier_ai_params->game_state->course_set = TRUE;
                }
                break;
            case TASK_STOP_ENGINE:
                soldier_ai_params->game_state->moving = FALSE;
                break;
            case TASK_START_ENGINE:
                soldier_ai_params->game_state->moving = TRUE;
                break;
            case TASK_STEER:
                if (soldier_ai_params->game_state->has_target) {
                    soldier_ai_params->game_state->on_target = TRUE;
                }
                break;
            case TASK_FIRE_TORPEDO:
                soldier_ai_params->game_state->fired = TRUE;
                break;
            case TASK_SEAL_LEAK:
                map_close(game, actor->map, soldier_ai_params->action_destination_x, soldier_ai_params->action_destination_y);
                break;
            default:
                break;
        }
    }
    soldier_ai_params->busy = FALSE;
    task_t new_task = rand() % __ROUTINE_TASKS;
    //soldier_switch_task(actor, soldier_ai_params, new_task);
    action = soldier_switch_task(actor, soldier_ai_params, new_task);
    *out_x = soldier_ai_params->destination_x;
    *out_y = soldier_ai_params->destination_y;
    return action;
}

void command_given(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    game_state_t * game_state = UNMAYBE(system_params);
    game_state->pending_task = (task_t) UNMAYBE(sender_params);
    game_state->new_task = TRUE;
}


void logic_update(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    game_state_t * game_state = UNMAYBE(system_params);
    
    if (game_state->new_task) {
        game_state->new_task = FALSE;
    } else {
        game_state->pending_task = TASK_NO_TASK;
    }
    game_state->new_task = FALSE;
    
    game_state->firing = FALSE;
    game_state->steering = FALSE;
    game_state->engine_monitored = FALSE;
    game_state->weapons_monitored = FALSE;
    game_state->periscope_watched = FALSE;
    game_state->sonar_listened = FALSE;
    game_state->sealing_leak = FALSE;
    game_state->stopping_engine = FALSE;
    game_state->starting_engine = FALSE;
    game_state->setting_course = FALSE;
}

void logic_startup(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    uint32_t tile_x, tile_y;
    int32_t x, y;
    int i;
    actor_t *actor;
    soldier_ai_params_t *soldier_ai_params;
    
    game_state_t * game_state = UNMAYBE(system_params);
    game_state->current_map = map_get_main_map(game);
    
    map_get_random_tile(game_state->current_map, TILE_CPT_QUARTERS, &tile_x, &tile_y);
    map_tile_center(game_state->current_map, tile_x, tile_y, &x, &y);
    game_state->captain = sys_actors_add_actor(game, game_state->current_map, ACTOR_CAPTAIN, x - ACTOR_SIZE / 2, y - ACTOR_SIZE / 2);
    game_state->captain->ai.get_action = MAYBIFY_FUNC(get_controller_action);
    game_state->captain->ai.ai_params = MAYBIFY(game_state);
    
    for (i = 0; i < NUM_SOLDIERS; i++) {
        map_get_random_tile(game_state->current_map, TILE_ANYWHERE, &tile_x, &tile_y);
        map_tile_center(game_state->current_map, tile_x, tile_y, &x, &y);
        actor = sys_actors_add_actor(game, game_state->current_map, ACTOR_SOLDIER, x - ACTOR_SIZE / 2, y - ACTOR_SIZE / 2);
        actor->ai.get_action = MAYBIFY_FUNC(get_soldier_action);
        soldier_ai_params = mem_alloc(sizeof(*soldier_ai_params));
        soldier_ai_params->task = TASK_IDLE;
        soldier_ai_params->destination_x = DESTINATION_NOT_SET;
        soldier_ai_params->destination_y = DESTINATION_NOT_SET;
        soldier_ai_params->time_left = 0;
        soldier_ai_params->anxiety = 0;
        soldier_ai_params->danger_level = NO_DANGER;
        soldier_ai_params->game_state = game_state;
        soldier_ai_params->busy = FALSE;
        actor->ai.ai_params = MAYBIFY(soldier_ai_params);
        actor->ai.ai_params_free = MAYBIFY_FUNC(mem_free);
    }
    text_add_printer(game, "status", 100, 300);
}

bool logic_start(game_t *game, system_t *system) {
    system->name = SYS_LOGIC_NAME;
    game_state_t * game_state = mem_alloc(sizeof(*game_state));
    game_state->controller_x = DESTINATION_NOT_SET;
    game_state->controller_y = DESTINATION_NOT_SET;
    game_state->pending_task = TASK_NO_TASK;
    game_state->new_task = FALSE;
    game_state->firing = FALSE;
    game_state->steering = FALSE;
    game_state->engine_monitored = FALSE;
    game_state->weapons_monitored = FALSE;
    game_state->periscope_watched = FALSE;
    game_state->sonar_listened = FALSE;
    game_state->sealing_leak = FALSE;
    game_state->stopping_engine = FALSE;
    game_state->starting_engine = FALSE;
    game_state->setting_course = FALSE;
    
    system->data = MAYBIFY(game_state);
    system->data_free = MAYBIFY_FUNC(mem_free);
    
    game_import_event(game, system, sdl_left_mouse_down);
    game_import_event(game, system, sdl_right_mouse_down);
    game_import_event(game, system, commands_send_command);
    game_register_hook(game, system, logic_startup, MAYBIFY(game_state), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, logic_update, MAYBIFY(game_state), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, update_controller, MAYBIFY(game_state), sdl_right_mouse_down, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, update_controller, MAYBIFY(game_state), sdl_left_mouse_down, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, command_given, MAYBIFY(game_state), commands_send_command, MAYBIFY_FUNC(NULL));
    
    return TRUE;
}
