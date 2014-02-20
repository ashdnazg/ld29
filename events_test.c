#include "system.h"
#include "macros.h"
#include "event.h"
#include "event_types.h"
#include "mem_wrap.h"
#include <stdlib.h>
#include <stdio.h>


IMPORTED_EVENTS
    DECLARE_EVENT(event1),
    DECLARE_EVENT(event2),
    DECLARE_EVENT(event3)
END_IMPORTED_EVENTS

void print_something(system_t *system, MAYBE(void *) params) {
    if (UNMAYBE(params) != NULL) {
        char *str = (char *) UNMAYBE(params);
        printf("%s\n", str);
    } else {
        printf("No params\n");
    }
}

int main(int argc, char* argv[]) {
    mem_wrap_init();
    system_t sys;
    events_map_t map;
    events_map_init(&map);
    system_init(&sys, "system");
    events_map_export(&map, "exevent");
    events_map_export(&map, "event1");
    events_map_export(&map, "event2");
    events_map_export(&map, "event3");
    events_map_import(&map, &sys, event1);
    events_map_import(&map, &sys, event2);
    events_map_import(&map, &sys, event3);
    //events_map_register_hook(&map, &sys, NULL, EVENT_NEW_FRAME);
    //events_map_register_hook(&map, &sys, NULL, EVENT_NEW_FRAME);
    //events_map_register_hook(&map, &sys, NULL, CUSTOM_EVENT(event2));
    events_map_register_hook(&map, &sys, print_something, EVENT_NEW_STEP);
    events_map_register_hook(&map, &sys, print_something, EVENT_NEW_STEP);
    events_map_register_hook(&map, &sys, print_something, EVENT_NEW_STEP);
    events_map_register_hook(&map, &sys, print_something, EVENT_NEW_STEP);
    events_map_process_pending(&map);
    printf("processed\n");
    
    
    events_map_loop(&map);
    printf("ended loop\n");
    
    events_map_clean(&map);
    printf("events_cleaned\n");
    
    system_clean(&sys);
    printf("system_cleaned\n");

    mem_wrap_print_mallocs();
    return 0;
}
