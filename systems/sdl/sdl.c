#include "sdl.h"
#include <SDL2/SDL.h>
#include "core/system.h"
#include "core/event.h"
#include "core/macros.h"
#include "core/mem_wrap.h"
#include "core/builtin_events.h"
#include "core/game.h"
#include <stdint.h>
#include <stdio.h>
#include "sdl_video.h"


LOCAL_EVENTS
    CUSTOM_EVENT(sdl_check_input)
END_LOCAL_EVENTS

void key_pressed(game_t *game, system_t *system, SDL_Keycode keycode) {
    switch (keycode) {
        case SDLK_p:
            game_push_event(game, system, EVENT_TOGGLE_PAUSE, MAYBIFY(NULL));
            break;
    }
}

void key_released(game_t *game, system_t *system, SDL_Keycode keycode) {

}

void sys_SDL_check_input(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    SDL_Event e;
    bool draw;
    int32_t time_to_next;
    sys_SDL_data_t *sys_SDL_data = (sys_SDL_data_t *) UNMAYBE(system_params);
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                game_push_event(game, system, EVENT_EXIT, MAYBIFY(NULL));
                return;
            case SDL_KEYDOWN:
                if (!(sys_SDL_data->key_states[e.key.keysym.scancode])) {
                    sys_SDL_data->key_states[e.key.keysym.scancode] = TRUE;
                    key_pressed(game, system, e.key.keysym.sym);
                }
                break;
            case SDL_KEYUP:
                if (sys_SDL_data->key_states[e.key.keysym.scancode]) {
                    sys_SDL_data->key_states[e.key.keysym.scancode] = FALSE;
                    key_released(game, system, e.key.keysym.sym);
                }
                break;
        }
    }
    time_to_next = (int32_t) (sys_SDL_data->next_frame_time - SDL_GetTicks());
    if(time_to_next <= 0) {
        sys_SDL_data->next_frame_time += STEP_INTERVAL;
        draw = TRUE;
        if(time_to_next < -(STEP_INTERVAL * SKIP_THRESHOLD))
        {
            if(sys_SDL_data->frames_skipped >= MAX_SKIP) {
                sys_SDL_data->next_frame_time = SDL_GetTicks();
                sys_SDL_data->frames_skipped = 0;
            } else {
                ++(sys_SDL_data->frames_skipped);
                draw = FALSE;
                printf("skipped\n");
            }
        } else {
            sys_SDL_data->frames_skipped = 0;
            if (!(game->paused)) {
                game_push_event(game, system, EVENT_NEW_STEP, MAYBIFY(NULL));
            }
            if (draw) {
                game_push_event(game, system, EVENT_NEW_FRAME, MAYBIFY(NULL));
            }
        }
        ++(sys_SDL_data->frames_this_second);
        if ((SDL_GetTicks() - sys_SDL_data->last_time) >= 1000){
            printf("frames: %d\n", sys_SDL_data->frames_this_second);
            sys_SDL_data->frames_this_second = 0;
            sys_SDL_data->last_time += 1000;
        }
    } else {
        SDL_Delay(1);
    }
    game_push_event(game, system, LOCAL_EVENT(sdl_check_input), MAYBIFY(NULL));
}

void sys_SDL_clean(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    sys_SDL_data_t *sys_SDL_data = (sys_SDL_data_t *) UNMAYBE(system_params);
    if (sys_SDL_data->ren != NULL) {
        SDL_DestroyRenderer(sys_SDL_data->ren);
    }
    if (sys_SDL_data->win != NULL) {
        SDL_DestroyWindow(sys_SDL_data->win);
    }
    SDL_Quit();
}


bool start(game_t *game, system_t *system) {
    system->name ="SDL";
    sys_SDL_data_t *sys_SDL_data = mem_alloc(sizeof(*sys_SDL_data));
    sys_SDL_data->win = NULL;
    sys_SDL_data->ren = NULL;
    sys_SDL_data->next_frame_time = SDL_GetTicks();
    sys_SDL_data->frames_skipped = 0;
    sys_SDL_data->frames_this_second = 0;
    sys_SDL_data->last_time = SDL_GetTicks();
    memset(sys_SDL_data->key_states, FALSE, sizeof(sys_SDL_data->key_states));
    
    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) == -1){
        return FALSE;
    }
    game_export_event(game, system, sdl_check_input, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, sys_SDL_clean, MAYBIFY(sys_SDL_data), EVENT_EXIT, MAYBIFY_FUNC(mem_free));
    game_register_hook(game, system, sys_SDL_check_input, MAYBIFY(sys_SDL_data), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, sys_SDL_check_input, MAYBIFY(sys_SDL_data), LOCAL_EVENT(sdl_check_input), MAYBIFY_FUNC(NULL));
    
    sys_SDL_data->win = SDL_CreateWindow(GAME_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAME_WIDTH * WINDOW_SCALE, GAME_HEIGHT * WINDOW_SCALE, SDL_WINDOW_SHOWN);
    if (sys_SDL_data->win == NULL) {
        return FALSE;
    }
    sys_SDL_data->ren = SDL_CreateRenderer(sys_SDL_data->win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sys_SDL_data->ren == NULL) {
        return FALSE;
    }
    SDL_RenderSetLogicalSize(sys_SDL_data->ren, GAME_WIDTH, GAME_HEIGHT);
    return TRUE;
}
