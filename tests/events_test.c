#include "core/system.h"
#include "core/macros.h"
#include "core/event.h"
#include "core/builtin_events.h"
#include "core/mem_wrap.h"
#include "core/game.h"
//#include "systems/logger/logger.h"
#include <stdlib.h>
#include <stdio.h>

LOCAL_EVENTS
    event1,
    event2,
    event3,
    exevent
END_LOCAL_EVENTS

void print_something(game_t *game, system_t *system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    if (UNMAYBE(sender_params) != NULL) {
        char *str = (char *) UNMAYBE(sender_params);
        printf("%s\n", str);
    } else {
        printf("No params\n");
    }
}

int main(int argc, char* argv[]) {
    printf("number of local events: %d\n", __LOCAL_EVENTS_COUNT);
    game_t game;
    system_t *sys = system_new();
    //system_t logger;
    game_init(&game);
    sys->name = "system1";
    //init_logger(&map, &logger);
    game_export_event(&game, sys, exevent, MAYBIFY_FUNC(NULL));
    game_export_event(&game, sys, event1, MAYBIFY_FUNC(NULL));
    game_export_event(&game, sys, event2, MAYBIFY_FUNC(NULL));
    game_export_event(&game, sys, event3, MAYBIFY_FUNC(NULL));
    game_register_hook(&game, sys, print_something, MAYBIFY(NULL), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(&game, sys, print_something, MAYBIFY(NULL), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(&game, sys, print_something, MAYBIFY(NULL), EVENT_START, MAYBIFY_FUNC(NULL));
    game_register_hook(&game, sys, print_something, MAYBIFY(NULL), EVENT_START, MAYBIFY_FUNC(NULL));
    game_start(&game);
    printf("processed\n");
    
    game_clean(&game);
    printf("events_cleaned\n");
    
    //system_free(sys);
    //system_clean(&logger);
    printf("system_cleaned\n");

    mem_wrap_print_mallocs();
    return 0;
}
