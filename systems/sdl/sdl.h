#ifndef __LOGGER_H__
#define __LOGGER_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "SDL2/SDL.h"
#include "core/system.h"
#include "core/game.h"

#define GAME_NAME "AESIS"
#define GAME_WIDTH 320
#define GAME_HEIGHT 240
#define WINDOW_SCALE 2


typedef struct sys_SDL_data_s {
    SDL_Window *win;
    SDL_Renderer *ren;
} sys_SDL_data_t;

bool start(game_t *game, system_t *system);


#ifdef __cplusplus
}
#endif

#endif