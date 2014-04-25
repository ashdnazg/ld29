#ifndef __LOGGER_H__
#define __LOGGER_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "SDL2/SDL.h"
#include "core/system.h"
#include "core/game.h"
#include <stdint.h>


#include "sdl_video.h"
#include "sdl_audio.h"

#define GAME_NAME "AESIS"
#define GAME_WIDTH 320
#define GAME_HEIGHT 240
#define WINDOW_SCALE 2
#define STEP_INTERVAL 17
#define SKIP_THRESHOLD 5
#define MAX_SKIP 5

#ifdef _WIN32
#define ASSETS_DIR "assets\\"
#else
#define ASSETS_DIR "assets/"
#endif

typedef struct sys_SDL_data_s {
    SDL_Window *win;
    SDL_Renderer *ren;
    uint32_t next_frame_time;
    int frames_skipped;
    int frames_this_second;
    uint32_t last_time;
    bool key_states[SDL_NUM_SCANCODES];
    uint32_t key_press_events[SDL_NUM_SCANCODES];
    uint32_t key_release_events[SDL_NUM_SCANCODES];
    render_manager_t render_manager;
    sound_manager_t sound_manager;
} sys_SDL_data_t;

bool start(game_t *game, system_t *system);


#ifdef __cplusplus
}
#endif

#endif
