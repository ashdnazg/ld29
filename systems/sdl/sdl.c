#include "sdl.h"
#include <SDL2/SDL.h>
#include "core/system.h"
#include "core/event.h"
#include "core/macros.h"
#include "core/mem_wrap.h"
#include "core/builtin_events.h"
#include "core/game.h"


LOCAL_EVENTS
    CUSTOM_EVENT(sdl_check_input)
END_LOCAL_EVENTS

void sys_SDL_check_input(events_queue_t *events_queue, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            push_event(events_queue, EVENT_EXIT, MAYBIFY(NULL));
            return;
        }
    }
    push_event(events_queue, LOCAL_EVENT(system, sdl_check_input), MAYBIFY(NULL));
}

void sys_SDL_clean(events_queue_t *events_queue, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
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
    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE) == -1){
        return FALSE;
    }
    game_register_hook(game, system, sys_SDL_clean, MAYBIFY(sys_SDL_data), EVENT_EXIT, MAYBIFY_FUNC(mem_free));
    game_register_hook(game, system, sys_SDL_check_input, MAYBIFY(NULL), EVENT_START, MAYBIFY_FUNC(NULL));
    
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
