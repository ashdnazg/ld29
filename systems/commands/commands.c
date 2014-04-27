#include "commands.h"
#include "core/mem_wrap.h"
#include "core/int_list.h"
#include "core/game.h"

#include <stdio.h>

LOCAL_EVENTS
    commands_send_command,
    sdl_pressed_number
END_LOCAL_EVENTS

void command_state_add_rule(command_state_t *state, command_state_t *to, const char *name, task_t task) {
    int i;
    for (i = 1; i < MAX_RULES; i++) {
        if (NULL == state->rules[i].target) {
            state->rules[i].target = to;
            state->rules[i].name = name;
            state->rules[i].task = task;
            return;
        }
    }
}

command_state_t * command_state_new(command_state_t *parent, command_state_t * head, const char *name) {
    int i;
    command_state_t *state = mem_alloc(sizeof(*state));
    link_init(&(state->states_link));
    for (i = 0; i < MAX_RULES; i++) {
        state->rules[i].target = NULL;
        state->rules[i].name = NULL;
        state->rules[i].task = TASK_NO_TASK;
    }
    if (NULL != parent) {
        command_state_add_rule(parent, state, name, TASK_NO_TASK);
    }
    if (NULL != head) {
        state->rules[0].target = head;
        state->rules[0].name = "back";
        state->rules[0].task = TASK_NO_TASK;
    }
    return state;
}

command_state_t * command_data_add_state(command_data_t *command_data, command_state_t *parent, const char *name) {
    command_state_t *state = command_state_new(parent, command_data->head, name);
    list_insert_tail(&(command_data->states), state);
    return state;
}


void command_data_free(command_data_t *command_data) {
    list_for_each(&(command_data->states), command_state_t *, command_state) {
        mem_free(command_state);
    }
    mem_free(command_data);
}

void print_commands(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    int i;
    char text_buffer[BUFFER_SIZE];
    command_data_t *command_data = (command_data_t *) UNMAYBE(system->data);
    if (command_data->changed) {
        text_clear_printer(game, "commands");
        for (i = 1; i < MAX_RULES; i++) {
            if (NULL == command_data->current->rules[i].target) {
                break;
            }
            sprintf(text_buffer, "%d( %s", i, command_data->current->rules[i].name);
            text_print_line(game, "commands", text_buffer);
        }
        if (NULL != command_data->current->rules[0].target)
        {
            sprintf(text_buffer, "0( %s", command_data->current->rules[0].name);
            text_print_line(game, "commands", text_buffer);
        }
        command_data->changed = FALSE;
    }
}

const char * get_task_sample(task_t task) {
    switch(task) {
        case TASK_LISTEN_SONAR:
            return "listen_sonar";
        case TASK_WATCH_PERISCOPE:
            return "watch_periscope";
        case TASK_MONITOR_ENGINE:
            return "monitor_engine";
        case TASK_MONITOR_WEAPONS:
            return "monitor_weapons";
        case TASK_SET_COURSE:
            return "set_course";
        case TASK_STOP_ENGINE:
            return "stop_engine";
        case TASK_START_ENGINE:
            return "start_engine";
        case TASK_STEER:
            return "steer";
        case TASK_FIRE_TORPEDO:
            return "fire_torpedo";
        case TASK_SEAL_LEAK:
            return "seal_leak";
        case TASK_PANIC:
            return "panic";
        default:
            printf("task has no sample: %d", task);
            exit(1);
    }
}

void choose_command(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    command_data_t *command_data = (command_data_t *) UNMAYBE(system->data);
    char c = (char) UNMAYBE(sender_params);
    int num;
    if (c < '0' && c > '9') {
        return;
    }
    if (command_data->voice != NULL) {
        return;
    }
    num = c - '0';
    if (NULL != command_data->current->rules[num].target) {
        if (command_data->current->rules[num].task != TASK_NO_TASK) {
            game_push_event(game, system, commands_send_command, MAYBIFY(command_data->current->rules[num].task));
            
            command_data->voice = sys_SDL_play_sample(game, get_task_sample(command_data->current->rules[num].task), 100, FALSE, (void **) &(command_data->voice));
        }
        command_data->current = command_data->current->rules[num].target;
        command_data->changed = TRUE;
    }
}

void command_init(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    text_add_printer(game, "commands", 100, 300);
}

bool commands_start(game_t *game, system_t *system) {
    system->name = SYS_COMMANDS_NAME;
    game_export_event(game, system, commands_send_command, MAYBIFY_FUNC(NULL));
    command_data_t *command_data = mem_alloc(sizeof(*command_data));
    command_data->voice = NULL;
    
    game_import_event(game, system, sdl_pressed_number);
    
    game_register_hook(game, system, choose_command, MAYBIFY(command_data), sdl_pressed_number, MAYBIFY_FUNC(NULL));
    
    list_init(&(command_data->states), command_state_t, states_link);
    system->data = MAYBIFY(command_data);
    system->data_free = MAYBIFY_FUNC(command_data_free);
    command_data->changed = TRUE;
    command_data->head = NULL;
    command_data->head = command_data_add_state(command_data, NULL, NULL);
    command_state_t *state = command_data_add_state(command_data, command_data->head, "Weapons");
    command_state_add_rule(state, command_data->head, "Monitor", TASK_MONITOR_WEAPONS);
    command_state_add_rule(state, command_data->head, "Fire!", TASK_FIRE_TORPEDO);
    state = command_data_add_state(command_data, command_data->head, "Engines");
    command_state_add_rule(state, command_data->head, "Monitor", TASK_MONITOR_ENGINE);
    command_state_add_rule(state, command_data->head, "Stop", TASK_STOP_ENGINE);
    command_state_add_rule(state, command_data->head, "Start", TASK_START_ENGINE);
    state = command_data_add_state(command_data, command_data->head, "Control");
    command_state_add_rule(state, command_data->head, "Man the Periscope", TASK_WATCH_PERISCOPE);
    command_state_add_rule(state, command_data->head, "Man the Sonar", TASK_LISTEN_SONAR);
    command_state_add_rule(state, command_data->head, "Head to Target", TASK_STEER);
    command_state_add_rule(state, command_data->head, "Head to Harbour", TASK_SET_COURSE);
    state = command_data_add_state(command_data, command_data->head, "Misc");
    command_state_add_rule(state, command_data->head, "Seal Leak", TASK_SEAL_LEAK);
    command_state_add_rule(state, command_data->head, "Don't Panic", TASK_PANIC);
    
    command_data->current = command_data->head;
    game_register_hook(game, system, command_init, MAYBIFY(NULL), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, print_commands, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    
    return TRUE;
}
