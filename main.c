#include "core/system.h"
#include "core/macros.h"
#include "core/event.h"
#include "core/builtin_events.h"
#include "core/mem_wrap.h"
#include "core/game.h"
#include "systems/sdl/sdl.h"
#include "systems/map/map.h"
#include "systems/actors/actors.h"
#include "systems/logic/logic.h"
#include "SDL2/SDL.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    game_t game;
    game_init(&game);
    if(game_add_system(&game, sdl_start)) {
        printf("loaded sdl\n");
    }
    if(game_add_system(&game, map_start)) {
        printf("loaded map\n");
    }
    if(game_add_system(&game, actors_start)) {
        printf("loaded actors\n");
    }
    if(game_add_system(&game, logic_start)) {
        printf("loaded logic\n");
    }
    game_load_systems(&game);
    
    game_start(&game);
    
    game_clean(&game);

    mem_wrap_print_mallocs();
    return 0;
}
