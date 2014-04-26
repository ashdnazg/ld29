#ifndef __MAP_H__
#define __MAP_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "core/system.h"
#include "core/game.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include "systems/sdl/sdl_video.h"

#define SPRITE_EXTENSION ".png"
#ifdef _WIN32
#define ASSETS_DIR "assets\\"
#else
#define ASSETS_DIR "assets/"
#endif

#define RGBA 4
#define TILE_TYPE_MASK   0x00FFFFFF
#define MAP_DEPTH 0
#define TILE_SIZE 10

#define COORD(map, x, y) ((y) * map->width + (x))

typedef enum tile_type_e {
    TILE_NOTHING       = 0x00FFFFFF,
    TILE_WEAPONS       = 0x000000FF,
    TILE_SONAR         = 0x00FF7FFF,
    TILE_CONTROL       = 0x00FF00FF,
    TILE_CREW_QUARTERS = 0x0000FF00,
    TILE_CPT_QUARTERS  = 0x00007F00,
    TILE_HOLD          = 0x00FFFF00,
    TILE_ENGINES       = 0x0000FFFF,
    TILE_PRESSURE_HULL = 0x007F7F00,
    TILE_OUTER_HULL    = 0x00FF0000,
    TILE_PASSAGE       = 0x007F7F7F,
    TILE_INTERIOR      = 0x00000000,
} tile_type_t;

typedef struct map_s {
    uint32_t height;
    uint32_t width;
    tile_type_t *matrix;
    MAYBE(renderable_t *) *renderables;
} map_t;

bool map_start(game_t *game, system_t *system);

map_t * map_new(const char *name);
void map_init(map_t *map, const char *name);
void map_clean(map_t *map);
void map_free(map_t *map);


#ifdef __cplusplus
}
#endif

#endif
