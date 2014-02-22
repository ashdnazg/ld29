#include "system.h"
#include "macros.h"
#include "event.h"
#include "builtin_events.h"
#include "mem_wrap.h"
#include "logger.h"
#include <stdlib.h>
#include <stdio.h>


IMPORTED_EVENTS
    CUSTOM_EVENT(event1),
    CUSTOM_EVENT(event2),
    CUSTOM_EVENT(event3)
END_IMPORTED_EVENTS

void print_something(events_queue_t *events_queue, system_t *system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    if (UNMAYBE(sender_params) != NULL) {
        char *str = (char *) UNMAYBE(sender_params);
        printf("%s\n", str);
    } else {
        printf("No params\n");
    }
}

int main(int argc, char* argv[]) {
    mem_wrap_init();
    system_t sys;
    system_t logger;
    events_map_t map;
    events_map_init(&map);
    system_init(&sys, "system");
    init_logger(&map, &logger);
    events_map_export(&map, "exevent", MAYBIFY_FUNC(NULL));
    events_map_export(&map, "event1", MAYBIFY_FUNC(NULL));
    events_map_export(&map, "event2", MAYBIFY_FUNC(NULL));
    events_map_export(&map, "event3", MAYBIFY_FUNC(NULL));
    events_map_import(&map, &sys, event1);
    events_map_import(&map, &sys, event2);
    events_map_import(&map, &sys, event3);
    events_map_register_hook(&map, &sys, print_something, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    events_map_register_hook(&map, &sys, print_something, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    events_map_register_hook(&map, &sys, print_something, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    events_map_register_hook(&map, &sys, print_something, MAYBIFY(NULL), EVENT_NEW_STEP, MAYBIFY_FUNC(NULL));
    events_map_process_pending(&map);
    printf("processed\n");
    
    
    events_map_loop(&map);
    printf("ended loop\n");
    
    events_map_clean(&map);
    printf("events_cleaned\n");
    
    system_clean(&sys);
    system_clean(&logger);
    printf("system_cleaned\n");

    mem_wrap_print_mallocs();
    return 0;
}
