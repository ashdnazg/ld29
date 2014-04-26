#ifndef __LOGIC_H__
#define __LOGIC_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "core/system.h"
#include "core/game.h"
#include "core/int_list.h"

#include "systems/map/map.h"

#include <stdint.h>

#define SYS_LOGIC_NAME "logic"
#define DESTINATION_NOT_SET UINT32_MAX

#define TASK_MAX_TIME (20 * 60)
#define TASK_MIN_TIME (5 * 60)

#define NUM_SOLDIERS 30

typedef enum task_e {
    TASK_IDLE,
    TASK_EAT,
    TASK_SLEEP,
    TASK_LISTEN_SONAR,
    TASK_WATCH_PERISCOPE,
    TASK_SUPERVISE_ENGINE,
    __ROUTINE_TASKS,
    TASK_NO_TASK,
    TASK_OPEN_PASSAGE,
    TASK_FIRE_TORPEDO,
    TASK_CLOSE_PASSAGE,
    TASK_PANIC,
    TASK_REGRET
} task_t;

typedef enum danger_level_e {
    NO_DANGER,
    GENERAL_DANGER,
    DIRECT_DANGER
} danger_level_t;

typedef enum direction_e {
    DIR_N,
    DIR_NE,
    DIR_E,
    DIR_SE,
    DIR_S,
    DIR_SW,
    DIR_W,
    DIR_NW
} direction_t;

typedef struct soldier_ai_params_s {
    task_t task;
    uint32_t destination_x;
    uint32_t destination_y;
    uint32_t time_left;
    uint32_t anxiety;
    danger_level_t danger_level;
} soldier_ai_params_t;

typedef struct game_state_s {
    uint32_t controller_x;
    uint32_t controller_y;
    map_t *current_map;

} game_state_t;

bool logic_start(game_t *game, system_t *system);


#ifdef __cplusplus
}
#endif

#endif
