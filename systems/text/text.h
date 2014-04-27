#ifndef __TEXT_H__
#define __TEXT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct text_printer_s text_printer_t;
typedef struct text_data_s text_data_t;

#include "core/int_list.h"
#include "core/asset_cache.h"
#include "core/game.h"
#include "systems/sdl/sdl_video.h"

#define TEXT_SCALE 2

#define TEXT_SIZE_X 8
#define TEXT_SIZE_Y 8

#define X_SPACING 1
#define Y_SPACING 1

#define TEXT_DEPTH 1000
#define FONT_NAME "font"

#define SYS_TEXT_NAME "text"

typedef struct text_line_s {
    link_t lines_link;
    int count;
    renderable_t **renderables;
} text_line_t;

struct text_printer_s {
    int x;
    int y;
    int num_lines;
    int size_x;
    int size_y;
    list_t lines;
};


bool text_start(game_t *game, system_t *system);

void text_add_printer(game_t *game, const char *printer_name, int x, int y);
void text_clear_printer(game_t *game, const char *printer_name);
void text_print_line(game_t *game, const char *printer_name, const char *text);

#ifdef __cplusplus
}
#endif

#endif 
