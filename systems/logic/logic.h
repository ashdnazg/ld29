#ifndef __LOGIC_H__
#define __LOGIC_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "core/system.h"
#include "core/game.h"
#include "core/int_list.h"

#include "systems/map/map.h"
#include "systems/actors/actors.h"
#include "systems/sdl/sdl_video.h"

#include <stdint.h>

#define SYS_LOGIC_NAME "logic"
#define DESTINATION_NOT_SET UINT32_MAX

#define TASK_MAX_TIME (10 * 60)
#define TASK_MIN_TIME (3 * 60)

#define SEAL_TIME (2 * 60)

#define NUM_SOLDIERS 20

#define MALFUNCTION_CHANCE 1000
#define FIX_CHANCE 400

#define SHIP_CHANCE 2000
#define DEPTH_CHARGE_CHANCE 2000

#define RUN_AWAY_CHANCE 4000

#define WHALE_CHANCE 5000
#define WHAM_CHANCE 2000

#define MESSAGE_DELAY (60 * 5)
#define TORPEDO_DELAY (60 * 2)

#define TORPEDO_DEPTH -500

typedef enum peril_e {
    NO_PERIL,
    PERIL_WHALE,
    PERIL_SHIP
} peril_t;

typedef enum task_e {
    TASK_EAT,
    TASK_SLEEP,
    TASK_LISTEN_SONAR,
    TASK_WATCH_PERISCOPE,
    TASK_MONITOR_ENGINE,
    TASK_MONITOR_WEAPONS,
    TASK_MONITOR_HELM,
    __ROUTINE_TASKS,
    TASK_IDLE,
    TASK_NO_TASK,
    TASK_SET_COURSE,
    TASK_STOP_ENGINE,
    TASK_START_ENGINE,
    TASK_STEER,
    TASK_FIRE_TORPEDO,
    TASK_SEAL_LEAK,
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

typedef struct game_state_s {
    uint32_t controller_x;
    uint32_t controller_y;
    actor_t *captain;
    map_t *current_map;
    bool new_task;
    task_t pending_task;
    peril_t current_peril;
    const char *message;
    int message_delay;
    int torpedo_delay;
    //tasks status
    bool firing;
    bool steering;
    bool engine_monitored;
    bool weapons_monitored;
    bool helm_monitored;
    bool periscope_watched;
    bool sonar_listened;
    bool sealing_leak;
    bool stopping_engine;
    bool starting_engine;
    bool setting_course;
    
    //submarine status
    bool course_set;
    bool has_target;
    bool on_target;
    bool leaking;
    bool moving;
    bool fired;
    bool weapons_malfunction;
    bool engine_malfunction;
    renderable_t *torpedo;
} game_state_t;

typedef struct soldier_ai_params_s {
    bool busy;
    task_t task;
    uint32_t destination_x;
    uint32_t destination_y;
    uint32_t action_destination_x;
    uint32_t action_destination_y;
    uint32_t time_left;
    uint32_t anxiety;
    danger_level_t danger_level;
    game_state_t *game_state;
} soldier_ai_params_t;

bool logic_captain_alive(game_t *game);
bool logic_start(game_t *game, system_t *system);

#ifdef __cplusplus
}
#endif

#endif
