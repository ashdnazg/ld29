#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "core/game.h"
#include "systems/logic/logic.h"
#include "systems/text/text.h"
#include "systems/sdl/sdl.h"

#define SYS_COMMANDS_NAME "commands"

#define MAX_RULES 10

typedef struct command_state_s command_state_t;

typedef struct command_rule_s {
    const char *name;
    command_state_t *target;
    task_t task;
} command_rule_t;

struct command_state_s {
    link_t states_link;
    command_rule_t rules[MAX_RULES];
};

typedef struct command_data_s {
    bool changed;
    command_state_t *head;
    command_state_t *current;
    list_t states;
    sample_playback_t *voice;
} command_data_t;

bool commands_start(game_t *game, system_t *system);

#ifdef __cplusplus
}
#endif

#endif 
