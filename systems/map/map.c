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
    return map_in_map(map, x, y) && tile_passable(map->matrix[COORD(map,x,y)]) && map->water_level[COORD(map, x, y)] < MAX_WATER_LEVEL;
}

bool map_in_water(map_t *map, int x, int y) {
    return map_in_map(map, x, y) && map->water_level[COORD(map, x, y)] != 0;
}

bool map_drowned(map_t *map, int x, int y) {
    return map_in_map(map, x, y) && map->water_level[COORD(map, x, y)] >= MAX_WATER_LEVEL;
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

void map_get_random_tile(map_t *map, tile_type_t type, uint32_t *out_x, uint32_t *out_y) {
    int count = 0;
    int i, j, choice;
    tile_coord_t *optional_tiles = mem_alloc(sizeof(*optional_tiles) * map->height * map->width);
    
    for (i = 0; i < map->height; i++) {
        for (j = 0; j < map->width; j++) {
            if (map->matrix[COORD(map, j, i)] == type || (type == TILE_ANYWHERE && map_tile_passable(map,j,i) && 
                                                          map->water_level[COORD(map, j, i)] == 0)) {
                optional_tiles[count].x = j;
                optional_tiles[count].y = i;
                count += 1;
            }
        }
    }
    if (count == 0) {
        *out_x = 0;
        *out_y = 0;
        mem_free(optional_tiles);
        return;
    }
    choice = rand() % count;
    *out_x = optional_tiles[choice].x;
    *out_y = optional_tiles[choice].y;
    mem_free(optional_tiles);
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

const char *get_tile_name(tile_type_t type) {
    const char *tile_name;
    switch(type) {
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
            printf("wrong colour: %x\n", type);
            exit(1);
    }
    return tile_name;
}

void map_update_tile(game_t *game, map_t *map, uint32_t x, uint32_t y, tile_type_t new_type) {
    const char *tile_name;
    map->matrix[COORD(map,x,y)] = new_type;
    MAYBE(renderable_t *) maybe_rend = map->renderables[COORD(map,x,y)];
    if (UNMAYBE(maybe_rend) != NULL) {
        renderable_free((renderable_t *) UNMAYBE(maybe_rend));
    }
    tile_name = get_tile_name(map->matrix[COORD(map, x, y)]);
    if (tile_name != NULL) {
        map->renderables[COORD(map, x, y)] = MAYBIFY(sys_SDL_add_renderable(game, tile_name, x * TILE_SIZE, y * TILE_SIZE, MAP_DEPTH));
    } else {
        map->renderables[COORD(map, x, y)] = MAYBIFY(NULL);
    }
}


bool map_reachable(map_t *map, uint32_t origin_x, uint32_t origin_y, uint32_t destination_x, uint32_t destination_y) {
    dijkstra_map_t *d_map = map_create_dijkstra(map, origin_x, origin_y);
    bool result = d_map->matrix[COORD(d_map, destination_x, destination_y)] != DIJKSTRA_IMPASSABLE;
    dijkstra_map_free(d_map);
    return result;
}

bool map_find_best_hatch(map_t *map, uint32_t origin_x, uint32_t origin_y, uint32_t *out_hatch_x, uint32_t *out_hatch_y) {
    int i,j;
    int hatch_x, hatch_y;
    int min_hatch = DIJKSTRA_IMPASSABLE;
    dijkstra_map_t *d_map_leak = mem_alloc(sizeof(*d_map_leak));
    int delta = 0;
    tile_coord_t *current_tile;
    heap_t heap;
    d_map_leak->matrix = mem_alloc(sizeof(*(d_map_leak->matrix)) * map->height * map->width);
    dijkstra_map_t *d_map_origin = map_create_dijkstra(map, origin_x, origin_y);
    d_map_leak->height = map->height;
    d_map_leak->width = map->width;
    memset(d_map_leak->matrix, DIJKSTRA_IMPASSABLE, sizeof(*(d_map_leak->matrix)) * map->height * map->width);
    heap_init(&heap, tile_coord_t, coords_link);
    
    
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            if (map->water_level[COORD(map,j,i)] >= MAX_WATER_LEVEL) {
                heap_insert(&heap, tile_coord_new(j, i), 0);
            }
        }
    }
    
    
    while(!heap_is_empty(&heap)) {
        current_tile = (tile_coord_t *) heap_delete_min(&heap);
        if (d_map_leak->matrix[COORD(d_map_leak, current_tile->x, current_tile->y)] > current_tile->coords_link.key) {
            d_map_leak->matrix[COORD(d_map_leak, current_tile->x, current_tile->y)] = current_tile->coords_link.key;
            //printf ("x: %d, y: %d, key: %d\n", current_tile->x, current_tile->y, current_tile->coords_link.key);
            for (i = -1; i <= 1; i++) {
                for (j = -1; j <= 1; j++) {
                    if (i == 0 && j == 0) {
                        continue;
                    }
                    if (d_map_leak->matrix[COORD(d_map_leak, current_tile->x + j, current_tile->y + i)] != DIJKSTRA_IMPASSABLE) {
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
    
    
    
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            if (d_map_origin->matrix[COORD(d_map_origin, j, i)] != DIJKSTRA_IMPASSABLE && 
                d_map_leak->matrix[COORD(d_map_leak, j, i)] < min_hatch &&
                map->matrix[COORD(map, j, i)] == TILE_OPEN_PASSAGE) {
                min_hatch = d_map_leak->matrix[COORD(d_map_leak, j, i)];
                hatch_x = j;
                hatch_y = i;
                
            }
        }
    }
    dijkstra_map_free(d_map_leak);
    dijkstra_map_free(d_map_origin);
    if (min_hatch == DIJKSTRA_IMPASSABLE) {
        return FALSE;
    } else {
        *out_hatch_x = hatch_x;
        *out_hatch_y = hatch_y;
        return TRUE;
    }
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
    mem_free(map->water_level);
    mem_free(map->water_delta);
    mem_free(map->water_renderables);
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
    map->water_level = mem_alloc(sizeof(*(map->water_level)) * im_h * im_w);
    map->water_delta = mem_alloc(sizeof(*(map->water_level)) * im_h * im_w);
    map->water_renderables = mem_alloc(sizeof(*(map->water_renderables)) * im_h * im_w);
    memset(map->water_level, 0, sizeof(*(map->water_level)) * im_h * im_w);
    memset(map->water_delta, 0, sizeof(*(map->water_level)) * im_h * im_w);
    memset(map->water_renderables, 0, sizeof(*(map->water_renderables)) * im_h * im_w);
    map->wait_diffuse = DIFFUSE_DELAY;
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


void set_water_level(game_t *game, map_t *map, uint32_t x, uint32_t y, int8_t level, bool leak) {
    char buffer[BUFFER_SIZE];
    int actual_level = level;
    if (map->water_level[COORD(map, x, y)] == level) {
        return;
    }
    if (!leak && level > MAX_WATER_LEVEL) {
        actual_level = MAX_WATER_LEVEL;
    }
    if (level < 0) {
        actual_level = 0;
    }
    map->water_level[COORD(map, x, y)] = actual_level;
    if (NULL != UNMAYBE(map->water_renderables[COORD(map, x, y)])) {
        renderable_free((renderable_t *) UNMAYBE(map->water_renderables[COORD(map, x, y)]));
    }
    if (actual_level / 8 > 0) {
        sprintf(buffer, "water_%d", actual_level / 8);
        map->water_renderables[COORD(map, x, y)] = MAYBIFY(sys_SDL_add_renderable(game, buffer, x * TILE_SIZE, y * TILE_SIZE, MAP_WATER_DEPTH));
    } else {
        map->water_renderables[COORD(map, x, y)] = MAYBIFY(NULL);
    }
}

void map_random_leak(game_t *game, map_t *map) {
    uint32_t tile_x, tile_y;
    
    map_get_random_tile(map, TILE_PRESSURE_HULL, &tile_x, &tile_y);
    map_update_tile(game, map, tile_x, tile_y, TILE_NOTHING);
    set_water_level(game, map, tile_x, tile_y, LEAK_WATER_LEVEL, TRUE);
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
            tile_name = get_tile_name(map->matrix[i * map->width + j]);
            if (tile_name != NULL) {
                map->renderables[COORD(map, j, i)] = MAYBIFY(sys_SDL_add_renderable(game, tile_name, j * TILE_SIZE, i * TILE_SIZE, MAP_DEPTH));
            } else {
                map->renderables[COORD(map, j, i)] = MAYBIFY(NULL);
            }
        }
    }
}

void diffuse_leak(game_t *game, map_t *map, uint32_t x, uint32_t y) {
    int i,j;
    for (i = -1; i <= 1; ++i) {
        for (j = -1; j <= 1; ++j) {
            if (map_tile_passable(map, x + j, y + i) && map->water_level[COORD(map,x+j,y+i)] < MAX_WATER_LEVEL) {
                set_water_level(game, map, x + j, y + i, map->water_level[COORD(map,x+j,y+i)] + 4, FALSE);
            }
        }
    }
}

void diffuse_water(game_t *game, map_t *map, uint32_t x, uint32_t y) {
    int i,j;
    for (i = -1; i <= 1; ++i) {
        for (j = -1; j <= 1; ++j) {
            if (map_tile_passable(map, x + j, y + i)) {
                map->water_delta[COORD(map, x+j, y+i)] += map->water_level[COORD(map,x,y)] / 16;
                map->water_delta[COORD(map, x, y)] -= map->water_level[COORD(map,x,y)] / 16;
            }
        }
    } 
}

void map_update_leaks(game_t *game, system_t * system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    int i,j;
    map_t *map = (map_t *) UNMAYBE(system_params);
    if (map->wait_diffuse > 0) {
        --(map->wait_diffuse);
        return;
    }
    
    map->wait_diffuse = DIFFUSE_DELAY;
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            if (map->water_level[COORD(map,j,i)] == LEAK_WATER_LEVEL) {
                diffuse_leak(game, map, j, i);
            }
        }
    }
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            if (map->water_level[COORD(map,j,i)] > 16) {
                diffuse_leak(game, map, j, i);
            }
        }
    }
    for (i = 0; i < map->height; ++i) {
        for (j = 0; j < map->width; ++j) {
            if (map_tile_passable(map, j, i)) {
                set_water_level(game, map, j, i, map->water_level[COORD(map,j,i)] + map->water_delta[COORD(map,j,i)], FALSE);
                map->water_delta[COORD(map,j,i)] = -1;
            }
        }
    }
}

void map_close(game_t *game, map_t *map, uint32_t x, uint32_t y) {
    if (!map_in_map(map, x, y) || (map->matrix[COORD(map,x,y)] != TILE_OPEN_PASSAGE &&  map->matrix[COORD(map,x,y)] != TILE_CLOSED_PASSAGE)) {
        return;
    }
    if (map->matrix[COORD(map,x,y)] == TILE_OPEN_PASSAGE) {
        map_update_tile(game, map, x, y, TILE_CLOSED_PASSAGE);
        set_water_level(game, map, x, y, 0, FALSE);
    }
}

bool map_start(game_t *game, system_t *system) {
    system->name = SYS_MAP_NAME;
    map_t *main_map = map_new("submarine_plan");
    system->data = MAYBIFY(main_map);
    system->data_free = MAYBIFY_FUNC(map_free);
    
    game_register_hook(game, system, map_init_graphics, MAYBIFY(main_map), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(game, system, map_update_leaks, MAYBIFY(main_map), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    
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
