#ifndef __SDL_H__
#define __SDL_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <SDL2/SDL.h>
#include "core/system.h"
#include "core/game.h"
#include <stdint.h>


#include "sdl_video.h"
#include "sdl_audio.h"

#define SYS_SDL_NAME "SDL"

#define GAME_NAME "LD29"
#define GAME_WIDTH 800
#define GAME_HEIGHT 600
#define WINDOW_SCALE 1
#define STEP_INTERVAL 17
#define SKIP_THRESHOLD 5
#define MAX_SKIP 5

#define CAMERA_STEP 20

#ifdef _WIN32
#define ASSETS_DIR "assets\\"
#else
#define ASSETS_DIR "assets/"
#endif

typedef struct sys_sdl_mouse_down_data_s {
    int32_t x;
    int32_t y;
} sys_sdl_mouse_down_data_t;

typedef struct sys_sdl_data_s {
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
} sys_sdl_data_t;

bool sdl_start(game_t *game, system_t *system);

renderable_t * sys_SDL_add_renderable(game_t *game, const char *sprite_name, int x, int y, int depth);

#ifdef __cplusplus
}
#endif

#endif
