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
#define DESTINATION_NOT_SET -1

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
    map_t *current_map;
} game_state_t;

bool logic_start(game_t *game, system_t *system);


#ifdef __cplusplus
}
#endif

#endif
