#include "core/macros.h"
#include "core/mem_wrap.h"
#include "map.h"
#include "stb_image.h"
#include "systems/sdl/sdl.h"
#include "systems/sdl/sdl_video.h"

#include <SDL2/sdl.h>
#include <stdint.h>


void map_clean(map_t *map) {
    mem_free(map->matrix);
    mem_free(map->renderables);
}

void map_free(map_t *map) {
    map_clean(map);
    mem_free(map);
}

void map_init(map_t *map, const char *name) {
    int im_w, im_h, i , j;
    uint32_t tile_id;
    unsigned char *image = NULL;
    char path_buffer[BUFFER_SIZE];
    sprintf(path_buffer, "%s%s%s", ASSETS_DIR, name, SPRITE_EXTENSION);
    image = stbi_load(path_buffer, &im_w, &im_h, NULL, RGBA);
    //exit_on_stbi_error(image);
    map->height = im_h;
    map->width = im_w;
    map->matrix = mem_alloc(sizeof(*(map->matrix)) * im_h * im_w);
    map->renderables = mem_alloc(sizeof(*(map->renderables)) * im_h * im_w);
    for (i = 0; i < im_h; ++i) {
        for (j = 0; j < im_w; ++j) {
            tile_id = *((uint32_t *) &(image[i * im_w * 4 + j * 4])) & TILE_TYPE_MASK;
            switch(tile_id) {
                case TILE_NOTHING       :
                case TILE_WEAPONS       :
                case TILE_SONAR         :
                case TILE_CONTROL       :
                case TILE_CREW_QUARTERS :
                case TILE_CPT_QUARTERS  :
                case TILE_HOLD          :
                case TILE_ENGINES       :
                case TILE_PRESSURE_HULL :
                case TILE_OUTER_HULL    :
                case TILE_PASSAGE       :
                    map->matrix[i*im_w + j] = tile_id;
                    break;
                default:
                    printf("wrong colour: %x, %d, %d\n", tile_id, i, j);
                    map->matrix[i*im_w + j] = TILE_NOTHING;
                    break;
            }
            
        }
    }
    free(image);
}

map_t * map_new(const char *name) {
    map_t *map = mem_alloc(sizeof(*map));
    map_init(map, name);
    return map;
}

void map_init_graphics(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    map_t *map = (map_t *) UNMAYBE(system_params);
    
    int i, j;
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            switch(map->matrix[i * map->width + j]) {
                case TILE_WEAPONS       :
                case TILE_SONAR         :
                case TILE_CONTROL       :
                case TILE_CREW_QUARTERS :
                case TILE_CPT_QUARTERS  :
                case TILE_HOLD          :
                case TILE_ENGINES       :
                case TILE_PRESSURE_HULL :
                    map->renderables[i * map->width + j] = MAYBIFY(sys_SDL_add_renderable(game, "tile_pressure_hull", j * TILE_SIZE, i * TILE_SIZE, MAP_DEPTH));
                    printf("k");
                    break;
                case TILE_OUTER_HULL    :
                    map->renderables[i * map->width + j] = MAYBIFY(sys_SDL_add_renderable(game, "tile_outer_hull", j * TILE_SIZE, i  * TILE_SIZE , MAP_DEPTH));
                    printf("b");
                    break;
                case TILE_PASSAGE       :
                    map->renderables[i * map->width + j] = MAYBIFY(sys_SDL_add_renderable(game, "tile_passage", j * TILE_SIZE, i * TILE_SIZE, MAP_DEPTH));
                    printf("g");
                    break;
                case TILE_NOTHING       :
                    map->renderables[i * map->width + j] = MAYBIFY(NULL);
                    printf("w");
                    break;
                default:
                    printf("wrong colour: %x, %d, %d\n", map->matrix[i * map->width + j], i, j);
                    exit(1);
            }
        }
        printf("\n");
    }
}

bool map_start(game_t *game, system_t *system) {
    system->name ="map";
    // game_import_component(game, system, sdl_renderable);
    // game_import_event(game, system, sdl_add_renderable);
    map_t *main_map = map_new("submarine_plan");
    system->data = MAYBIFY(main_map);
    system->data_free = MAYBIFY_FUNC(map_free);
    
    game_register_hook(game, system, map_init_graphics, MAYBIFY(main_map), EVENT_START, MAYBIFY_FUNC(NULL));
    
    return TRUE;
}

//map_t * load_map(const char *name) {

    // bitmap = SDL_CreateRGBSurfaceFrom(image, im_w, im_h, RGBA * 8, RGBA * im_w,
                                   // RMASK, GMASK, BMASK, AMASK);
    // exit_on_SDL_error(bitmap);
    // texture = SDL_CreateTextureFromSurface(r_manager->renderer, bitmap);
    
    // SDL_FreeSurface(bitmap);
    // free(image);
    // asset_cache_add(&(r_manager->textures), texture, path);
    // return map;
// }
