#include "core/macros.h"
#include "core/mem_wrap.h"
#include "core/system.h"
#include "core/game.h"
#include "core/int_pheap.h"

#include "map.h"
#include "stb_image.h"
#include "systems/sdl/sdl.h"
#include "systems/sdl/sdl_video.h"

#include <SDL2/sdl.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

bool tile_passable(tile_type_t type) {
    switch(type) {
        case TILE_WEAPONS       :
        case TILE_SONAR         :
        case TILE_CONTROL       :
        case TILE_CREW_QUARTERS :
        case TILE_CPT_QUARTERS  :
        case TILE_HOLD          :
        case TILE_ENGINES       :
        case TILE_OPEN_PASSAGE  :
            return TRUE;
        case TILE_NOTHING       :
        case TILE_INTERIOR      :
        case TILE_INTERNAL_WALL :
        case TILE_CLOSED_PASSAGE:
        case TILE_OUTER_HULL    :
        case TILE_PRESSURE_HULL :
            return FALSE;
        default:
            printf("wrong tile type: %d\n", type);
            exit(1);
            break;
    }
}


bool map_in_map(map_t *map, int x, int y) {
        return (x >= 0 && y >= 0 && x <= map->width && y <= map->height);
}

bool map_tile_passable(map_t *map, int x, int y) {
    return map_in_map(map, x, y) && tile_passable(map->matrix[COORD(map,x,y)]);
}


tile_coord_t * tile_coord_new(uint32_t origin_x, uint32_t origin_y) {
    tile_coord_t *tc = mem_alloc(sizeof(*tc));
    heap_link_init(tc, coords_link);
    tc->x = origin_x;
    tc->y = origin_y;
    return tc;
}
void dijkstra_map_free(dijkstra_map_t *d_map) {
    mem_free(d_map->matrix);
    mem_free(d_map);
}

dijkstra_map_t * map_create_dijkstra(map_t *map, uint32_t origin_x, uint32_t origin_y) {
    int i,j;
    int delta = 0;
    dijkstra_map_t *d_map = mem_alloc(sizeof(*d_map));
    d_map->height = map->height;
    d_map->width = map->width;
    d_map->matrix = mem_alloc(sizeof(*(d_map->matrix)) * map->height * map->width);
    memset(d_map->matrix, DIJKSTRA_IMPASSABLE, sizeof(*(d_map->matrix)) * map->height * map->width);
    tile_coord_t *current_tile;
    heap_t heap;
    heap_init(&heap, tile_coord_t, coords_link);
    heap_insert(&heap, tile_coord_new(origin_x, origin_y), 0);
    
    while(!heap_is_empty(&heap)) {
        current_tile = (tile_coord_t *) heap_delete_min(&heap);
        if (d_map->matrix[COORD(d_map, current_tile->x, current_tile->y)] > current_tile->coords_link.key) {
            d_map->matrix[COORD(d_map, current_tile->x, current_tile->y)] = current_tile->coords_link.key;
            //printf ("x: %d, y: %d, key: %d\n", current_tile->x, current_tile->y, current_tile->coords_link.key);
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) {
                        continue;
                    }
                    if (d_map->matrix[COORD(d_map, current_tile->x + j, current_tile->y + i)] != DIJKSTRA_IMPASSABLE) {
                        continue;
                    }
                    if (!map_tile_passable(map, current_tile->x + j, current_tile->y + i)) {
                        continue;
                    }
                    if (i == 0 || j == 0) {
                        delta = 2;
                    } else {
                        delta = 3;
                    }
                    heap_insert(&heap, tile_coord_new(current_tile->x + j, current_tile->y + i), current_tile->coords_link.key + delta);
                }
            }
        }
        mem_free(current_tile); 
    }
    return d_map;
}


bool map_reachable(map_t *map, uint32_t origin_x, uint32_t origin_y, uint32_t destination_x, uint32_t destination_y) {
    dijkstra_map_t *d_map = map_create_dijkstra(map, origin_x, origin_y);
    bool result = d_map->matrix[COORD(d_map, destination_x, destination_y)] != DIJKSTRA_IMPASSABLE;
    dijkstra_map_free(d_map);
    return result;
}

map_t *map_get_main_map(game_t *game) {
    system_t *sys_map = game_get_system(game, SYS_MAP_NAME);
    map_t *main_map = (map_t *) UNMAYBE(sys_map->data);
    assert(NULL != main_map);
    return main_map;
}

void map_clean(map_t *map) {
    //int i,j;
    mem_free(map->matrix);
    // for (i = 0; i < map->height; ++i) {
        // for (j = 0; j < map->width; ++j) {
            // if (NULL != UNMAYBE(map->renderables[COORD(map, j, i)])) {
                // renderable_free((renderable_t *) UNMAYBE(map->renderables[COORD(map, j, i)]));
            // }
        // }
    // }
    mem_free(map->renderables);
}

bool map_rect_passable(map_t *map, int left, int top, int width, int height) {
    uint32_t tile_x, tile_y;
    if(!map_translate_coordinates(map, left, top, &tile_x, &tile_y)) {
        return FALSE;
    }
    if (!map_tile_passable(map, tile_x, tile_y)) {
        return FALSE;
    }
    if(!map_translate_coordinates(map, left + width - 1, top, &tile_x, &tile_y)) {
        return FALSE;
    }
    if (!map_tile_passable(map, tile_x, tile_y)) {
        return FALSE;
    }
    if(!map_translate_coordinates(map, left, top + height - 1, &tile_x, &tile_y)) {
        return FALSE;
    }
    if (!map_tile_passable(map, tile_x, tile_y)) {
        return FALSE;
    }
    if(!map_translate_coordinates(map, left + width -1 , top + height - 1, &tile_x, &tile_y)) {
        return FALSE;
    }
    if (!map_tile_passable(map, tile_x, tile_y)) {
        return FALSE;
    }
    return TRUE;
}

bool map_tile_center(map_t *map, uint32_t x, uint32_t y, int32_t *out_sdl_x, int32_t *out_sdl_y) {
    if (!map_in_map(map, x, y)) {
        return FALSE;
    }
    *out_sdl_x = x * TILE_SIZE + TILE_SIZE / 2;
    *out_sdl_y = y * TILE_SIZE + TILE_SIZE / 2;
    return TRUE;
}

bool map_translate_coordinates(map_t *map, int x, int y, uint32_t *out_map_x, uint32_t *out_map_y) {
    if(x < 0 || y < 0 || x > map->width * TILE_SIZE || y > map->height * TILE_SIZE) {
        return FALSE;
    }
    *out_map_x = x / TILE_SIZE;
    *out_map_y = y / TILE_SIZE;
    return TRUE;
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
                case TILE_NOTHING        :
                case TILE_WEAPONS        :
                case TILE_SONAR          :
                case TILE_CONTROL        :
                case TILE_CREW_QUARTERS  :
                case TILE_CPT_QUARTERS   :
                case TILE_HOLD           :
                case TILE_ENGINES        :
                case TILE_PRESSURE_HULL  :
                case TILE_OUTER_HULL     :
                case TILE_OPEN_PASSAGE   :
                case TILE_CLOSED_PASSAGE :
                case TILE_INTERNAL_WALL  :
                case TILE_INTERIOR       :
                    map->matrix[COORD(map, j, i)] = tile_id;
                    break;
                default:
                    printf("wrong colour: %x, %d, %d\n", tile_id, i, j);
                    map->matrix[COORD(map, j, i)] = TILE_NOTHING;
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
    const char *tile_name = NULL;
    int i, j;
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            switch(map->matrix[i * map->width + j]) {
                case TILE_WEAPONS       :
                    tile_name = "tile_weapons";
                    break;
                case TILE_SONAR         :
                    tile_name = "tile_sonar";
                    break;
                case TILE_CONTROL       :
                    tile_name = "tile_control";
                    break;
                case TILE_CREW_QUARTERS :
                    tile_name = "tile_crew_quarters";
                    break;
                case TILE_CPT_QUARTERS  :
                    tile_name = "tile_cpt_quarters";
                    break;
                case TILE_HOLD          :
                    tile_name = "tile_hold";
                    break;
                case TILE_ENGINES       :
                    tile_name = "tile_engines";
                    break;
                case TILE_PRESSURE_HULL :
                    tile_name = "tile_pressure_hull";
                    break;
                case TILE_OUTER_HULL    :
                    tile_name = "tile_outer_hull";
                    break;
                case TILE_OPEN_PASSAGE  :
                    tile_name = "tile_open_passage";
                    break;
                case TILE_CLOSED_PASSAGE :
                    tile_name = "tile_closed_passage";
                    break;
                case TILE_INTERIOR       :
                    tile_name = "tile_interior";
                    break;
                case TILE_INTERNAL_WALL  :
                    tile_name = "tile_internal_wall";
                    break;
                case TILE_NOTHING        :
                    tile_name = NULL;
                    break;
                default:
                    printf("wrong colour: %x, %d, %d\n", map->matrix[COORD(map, j, i)], i, j);
                    exit(1);
            }
            if (tile_name != NULL) {
                map->renderables[COORD(map, j, i)] = MAYBIFY(sys_SDL_add_renderable(game, tile_name, j * TILE_SIZE, i * TILE_SIZE, MAP_DEPTH));
            } else {
                map->renderables[COORD(map, j, i)] = MAYBIFY(NULL);
            }
        }
    }
}

bool map_start(game_t *game, system_t *system) {
    system->name = SYS_MAP_NAME;
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
