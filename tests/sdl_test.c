#include "core/system.h"
#include "core/macros.h"
#include "core/event.h"
#include "core/builtin_events.h"
#include "core/mem_wrap.h"
#include "core/game.h"
#include "systems/sdl/sdl.h"
#include "SDL2/SDL.h"
//#include "systems/logger/logger.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    mem_wrap_init();
    game_t game;
    game_init(&game);
    system_t *sys = system_new();
    start(&game, sys);
    game_load_systems(&game);
    list_insert_tail(&(game.systems), sys);
    
    game_start(&game);
    
    game_clean(&game);
    
    system_free(sys);

    mem_wrap_print_mallocs();
    return 0;
}
