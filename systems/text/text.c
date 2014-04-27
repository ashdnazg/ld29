#include "text.h"
#include "systems/sdl/sdl.h"
#include "systems/sdl/sdl_video.h"
#include "core/game.h"
#include "core/mem_wrap.h"
#include "core/macros.h"
#include "core/int_list.h"
#include "core/asset_cache.h"
#include <string.h>



void line_free(text_line_t * line, bool clear) {
    int i;
    link_remove_from_list(&(line->lines_link));
    if (clear) {
        for (i = 0; i < line->count; i++) {
            renderable_free(line->renderables[i]);
        }
    }
    mem_free(line->renderables);
    mem_free(line);
}


void text_printer_init(text_printer_t *t_printer, int x, int y) {
    t_printer->x = x;
    t_printer->y = y;
    t_printer->num_lines = 0;
    t_printer->size_x = TEXT_SIZE_X;
    t_printer->size_y = TEXT_SIZE_Y;
    list_init(&(t_printer->lines), text_line_t, lines_link);
}

text_printer_t * text_printer_new(int x, int y) {
    text_printer_t *t_printer = mem_alloc(sizeof(*t_printer));
    text_printer_init(t_printer, x, y);
    return t_printer;
}

void text_printer_free(text_printer_t *t_printer) {
    list_for_each(&(t_printer->lines), text_line_t *, line) {
        line_free(line, FALSE);
    }
    mem_free(t_printer);
}

bool text_start(game_t *game, system_t *system) {
    system->name = SYS_TEXT_NAME;
    asset_cache_t *printers = asset_cache_new(MAYBIFY_FUNC(text_printer_free));
    //text_printer_t *t_printer = text_printer_new();
    sys_SDL_load_sheet(game, FONT_NAME, TEXT_SIZE_X, TEXT_SIZE_Y, 0);
    system->data = MAYBIFY(printers);
    system->data_free = MAYBIFY_FUNC(asset_cache_free);
    return TRUE;
}

void text_printer_clear(text_printer_t *t_printer) {
    list_for_each(&(t_printer->lines), text_line_t *, line) {
        line_free(line, TRUE);
    }
    t_printer->num_lines = 0;
}

void text_printer_print(game_t *game, text_printer_t *t_printer, const char *text) {
    int temp_x = t_printer->x;
    int temp_y = t_printer->y + t_printer->num_lines * (t_printer->size_y + Y_SPACING);
    int i;
    int len = strlen(text);
    char name_buffer[BUFFER_SIZE];
    text_line_t * line = mem_alloc(sizeof(*line));
    line->count = len;
    line->renderables = mem_alloc(len * sizeof(renderable_t *));
    link_init(&(line->lines_link));
    for (i = 0;i < len;++i){
        sprintf(name_buffer, "%s%03d", FONT_NAME, text[i]);
        temp_x += t_printer->size_x + X_SPACING;
        line->renderables[i] = sys_SDL_add_renderable(game, name_buffer, temp_x, temp_y, TEXT_DEPTH);
        line->renderables[i]->offset = FALSE;
    }
    (t_printer->num_lines)++;
    list_insert_tail(&(t_printer->lines), line);
}

void text_clear_printer(game_t *game, const char *printer_name) {
    system_t *sys_text = game_get_system(game, SYS_TEXT_NAME);
    asset_cache_t *printers = (asset_cache_t *) UNMAYBE(sys_text->data);
    MAYBE(text_printer_t *) maybe_t_printer = asset_cache_get(printers, printer_name);
    if (NULL == UNMAYBE(maybe_t_printer)) {
        printf("no such printer: %s", printer_name);
    }
    text_printer_t *t_printer = (text_printer_t *) UNMAYBE(maybe_t_printer);
    text_printer_clear(t_printer);
}

void text_print_line(game_t *game, const char *printer_name, const char *text) {
    system_t *sys_text = game_get_system(game, SYS_TEXT_NAME);
    asset_cache_t *printers = (asset_cache_t *) UNMAYBE(sys_text->data);
    MAYBE(text_printer_t *) maybe_t_printer = asset_cache_get(printers, printer_name);
    if (NULL == UNMAYBE(maybe_t_printer)) {
        printf("no such printer: %s",  printer_name);
    }
    text_printer_t *t_printer = (text_printer_t *) UNMAYBE(maybe_t_printer);
    text_printer_print(game, t_printer, text);
    
}

void text_add_printer(game_t *game, const char *printer_name, int x, int y) {
    system_t *sys_text = game_get_system(game, SYS_TEXT_NAME);
    asset_cache_t *printers = (asset_cache_t *) UNMAYBE(sys_text->data);
    text_printer_t *t_printer = text_printer_new(x, y);
    asset_cache_add(printers, t_printer, printer_name);
}


