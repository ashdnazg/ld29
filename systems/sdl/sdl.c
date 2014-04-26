#include "sdl.h"
#include "sdl_video.h"
#include "sdl_audio.h"

#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <assert.h>

#include "core/system.h"
#include "core/event.h"
#include "core/macros.h"
#include "core/mem_wrap.h"
#include "core/builtin_events.h"
#include "core/game.h"
#include "core/settings.h"
#include "core/tween.h"

LOCAL_EVENTS
    sdl_check_input,
    sdl_move_camera_up,
    sdl_move_camera_down,
    sdl_move_camera_left,
    sdl_move_camera_right,
    sdl_left_mouse_down,
    sdl_right_mouse_down
END_LOCAL_EVENTS

void move_camera_up(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    sys_sdl_data->render_manager.y_offset += CAMERA_STEP;
}

void move_camera_down(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    sys_sdl_data->render_manager.y_offset -= CAMERA_STEP;
}

void move_camera_left(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    sys_sdl_data->render_manager.x_offset += CAMERA_STEP;
}

void move_camera_right(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    sys_sdl_data->render_manager.x_offset -= CAMERA_STEP;
}

void key_pressed(game_t *game, system_t *system, SDL_Scancode scancode, sys_sdl_data_t *sys_sdl_data) {
    if (sys_sdl_data->key_press_events[scancode] != NO_EVENT) {
        game_trigger_event(game, system, sys_sdl_data->key_press_events[scancode], MAYBIFY(NULL));
    }
}

void key_released(game_t *game, system_t *system, SDL_Keycode scancode, sys_sdl_data_t *sys_sdl_data) {
    if (sys_sdl_data->key_release_events[scancode] != NO_EVENT) {
        game_trigger_event(game, system, sys_sdl_data->key_release_events[scancode], MAYBIFY(NULL));
    }
}

void check_input(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    SDL_Event e;
    bool draw;
    int32_t time_to_next;
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    sys_sdl_mouse_down_data_t *mouse_down_data = NULL;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                game_push_event(game, system, EVENT_EXIT, MAYBIFY(NULL));
                return;
            case SDL_KEYDOWN:
                if (!(sys_sdl_data->key_states[e.key.keysym.scancode])) {
                    sys_sdl_data->key_states[e.key.keysym.scancode] = TRUE;
                    key_pressed(game, system, e.key.keysym.scancode, sys_sdl_data);
                }
                break;
            case SDL_KEYUP:
                if (sys_sdl_data->key_states[e.key.keysym.scancode]) {
                    sys_sdl_data->key_states[e.key.keysym.scancode] = FALSE;
                    key_released(game, system, e.key.keysym.scancode, sys_sdl_data);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                mouse_down_data = mem_alloc(sizeof(*mouse_down_data));
                mouse_down_data->x = e.button.x - sys_sdl_data->render_manager.x_offset;
                mouse_down_data->y = e.button.y - sys_sdl_data->render_manager.y_offset;
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    game_trigger_event(game, system, sdl_left_mouse_down, MAYBIFY(mouse_down_data));
                } else if (e.button.button == SDL_BUTTON_RIGHT) {
                    game_trigger_event(game, system, sdl_right_mouse_down, MAYBIFY(mouse_down_data));
                }
                break;
        }
    }
    time_to_next = (int32_t) (sys_sdl_data->next_frame_time - SDL_GetTicks());
    if(time_to_next <= 0) {
        sys_sdl_data->next_frame_time += STEP_INTERVAL;
        draw = TRUE;
        if(time_to_next < -(STEP_INTERVAL * SKIP_THRESHOLD))
        {
            if(sys_sdl_data->frames_skipped >= MAX_SKIP) {
                sys_sdl_data->next_frame_time = SDL_GetTicks();
                sys_sdl_data->frames_skipped = 0;
            } else {
                ++(sys_sdl_data->frames_skipped);
                draw = FALSE;
                printf("skipped\n");
            }
        } else {
            sys_sdl_data->frames_skipped = 0;
            if (!(game->paused)) {
                game_trigger_event(game, system, EVENT_NEW_STEP, MAYBIFY(NULL));
            }
            if (draw) {
                game_trigger_event(game, system, EVENT_NEW_FRAME, MAYBIFY(NULL));
            }
        }
        ++(sys_sdl_data->frames_this_second);
        if ((SDL_GetTicks() - sys_sdl_data->last_time) >= 1000){
            printf("frames: %d\n", sys_sdl_data->frames_this_second);
            sys_sdl_data->frames_this_second = 0;
            sys_sdl_data->last_time += 1000;
        }
    } else {
        SDL_Delay(1);
    }
    game_push_event(game, system, sdl_check_input, MAYBIFY(NULL));
}

void sys_SDL_clean(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    sound_manager_clean(&(sys_sdl_data->sound_manager));
    if (sys_sdl_data->ren != NULL) {
        SDL_DestroyRenderer(sys_sdl_data->ren);
        render_manager_clean(&(sys_sdl_data->render_manager));
    }
    if (sys_sdl_data->win != NULL) {
        SDL_DestroyWindow(sys_sdl_data->win);
    }
    SDL_Quit();
}

void sys_SDL_draw(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(system_params);
    render_manager_animate(&(sys_sdl_data->render_manager));
    render_manager_draw(&(sys_sdl_data->render_manager));
}


renderable_t * sys_SDL_add_renderable(game_t *game, const char *sprite_name, int x, int y, int depth) {
    system_t *sys_sdl = game_get_system(game, SYS_SDL_NAME);
    sys_sdl_data_t *sys_sdl_data = (sys_sdl_data_t *) UNMAYBE(sys_sdl->data);
    return render_manager_create_renderable(&(sys_sdl_data->render_manager), sprite_name, x, y, depth);
}

void set_key_press_from_settings(sys_sdl_data_t *sys_sdl_data, const char *key_settings, SDL_Keycode default_key, uint32_t event_id) {
    MAYBE(char *) key_str = settings_get_string(key_settings);
    if(UNMAYBE(key_str) == NULL) {
        sys_sdl_data->key_press_events[SDL_GetScancodeFromKey(default_key)] = event_id;
    } else {
        sys_sdl_data->key_press_events[SDL_GetScancodeFromKey(((char*) UNMAYBE(key_str))[0])] = event_id;
        mem_free(UNMAYBE(key_str));
    }
}

void set_key_release_from_settings(sys_sdl_data_t *sys_sdl_data, const char *key_settings, SDL_Keycode default_key, uint32_t event_id) {
    MAYBE(char *) key_str = settings_get_string(key_settings);
    if(UNMAYBE(key_str) == NULL) {
        sys_sdl_data->key_release_events[SDL_GetScancodeFromKey(default_key)] = event_id;
    } else {
        sys_sdl_data->key_release_events[SDL_GetScancodeFromKey(((char*) UNMAYBE(key_str))[0])] = event_id;
        mem_free(UNMAYBE(key_str));
    }
}

bool sdl_start(game_t *game, system_t *system) {
    system->name = SYS_SDL_NAME;
    sys_sdl_data_t *sys_sdl_data = mem_alloc(sizeof(*sys_sdl_data));
    sys_sdl_data->win = NULL;
    sys_sdl_data->ren = NULL;
    sys_sdl_data->next_frame_time = SDL_GetTicks();
    sys_sdl_data->frames_skipped = 0;
    sys_sdl_data->frames_this_second = 0;
    sys_sdl_data->last_time = SDL_GetTicks();
    memset(sys_sdl_data->key_states, FALSE, sizeof(sys_sdl_data->key_states));
    memset(sys_sdl_data->key_press_events, NO_EVENT, sizeof(sys_sdl_data->key_press_events));
    memset(sys_sdl_data->key_release_events, NO_EVENT, sizeof(sys_sdl_data->key_release_events));
    sound_manager_init(&(sys_sdl_data->sound_manager));
    
    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) == -1){
        return FALSE;
    }
    system->data = MAYBIFY(sys_sdl_data);
    system->data_free = MAYBIFY_FUNC(mem_free);
    
    game_export_event(game, system, sdl_check_input, MAYBIFY_FUNC(NULL));
    game_export_event(game, system, sdl_move_camera_up, MAYBIFY_FUNC(NULL));
    game_export_event(game, system, sdl_move_camera_down, MAYBIFY_FUNC(NULL));
    game_export_event(game, system, sdl_move_camera_left, MAYBIFY_FUNC(NULL));
    game_export_event(game, system, sdl_move_camera_right, MAYBIFY_FUNC(NULL));
    game_export_event(game, system, sdl_left_mouse_down, MAYBIFY_FUNC(mem_free));
    game_export_event(game, system, sdl_right_mouse_down, MAYBIFY_FUNC(mem_free));
    
    game_register_hook(game, system, sys_SDL_clean, MAYBIFY(sys_sdl_data), EVENT_EXIT, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, check_input, MAYBIFY(sys_sdl_data), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, check_input, MAYBIFY(sys_sdl_data), sdl_check_input, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, sys_SDL_draw, MAYBIFY(sys_sdl_data), EVENT_NEW_FRAME, MAYBIFY_FUNC(NULL));
    
    game_register_hook(game, system, move_camera_up, MAYBIFY(sys_sdl_data), sdl_move_camera_up, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, move_camera_down, MAYBIFY(sys_sdl_data), sdl_move_camera_down, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, move_camera_left, MAYBIFY(sys_sdl_data), sdl_move_camera_left, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, move_camera_right, MAYBIFY(sys_sdl_data), sdl_move_camera_right, MAYBIFY_FUNC(NULL));
    
    sys_sdl_data->win = SDL_CreateWindow(GAME_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAME_WIDTH * WINDOW_SCALE, GAME_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);
    if (sys_sdl_data->win == NULL) {
        return FALSE;
    }
    sys_sdl_data->ren = SDL_CreateRenderer(sys_sdl_data->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sys_sdl_data->ren == NULL) {
        return FALSE;
    }
    SDL_RenderSetLogicalSize(sys_sdl_data->ren, GAME_WIDTH, GAME_HEIGHT);
    render_manager_init(&(sys_sdl_data->render_manager), sys_sdl_data->ren);

    set_key_press_from_settings(sys_sdl_data, "pause_key",          SDLK_p,     EVENT_TOGGLE_PAUSE);
    set_key_press_from_settings(sys_sdl_data, "camera_up_key",      SDLK_UP,    sdl_move_camera_up);
    set_key_press_from_settings(sys_sdl_data, "camera_down_key",    SDLK_DOWN,  sdl_move_camera_down);
    set_key_press_from_settings(sys_sdl_data, "camera_left_key",    SDLK_LEFT,  sdl_move_camera_left);
    set_key_press_from_settings(sys_sdl_data, "camera_right_key",   SDLK_RIGHT, sdl_move_camera_right);

    return TRUE;
}
