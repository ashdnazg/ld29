#include "logger.h"
#include "system.h"
#include "event.h"
#include "macros.h"
#include "builtin_events.h"
#include <stdlib.h>
#include <stdio.h>

IMPORTED_EVENTS
    CUSTOM_EVENT(event3)
END_IMPORTED_EVENTS

void logger_log(events_queue_t *events_queue, system_t *system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    if (UNMAYBE(sender_params) != NULL) {
        char *str = (char *) UNMAYBE(sender_params);
        printf("Log: %s\n", str);
    } else {
        printf("Log: Nothing\n");
    }
    push_event(events_queue, LOCAL_EVENT(system, event3), MAYBIFY(NULL));
}

void test_hook(events_queue_t *events_queue, system_t *system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    printf("event3\n");
}

void init_logger(events_map_t *event_map, system_t *system) {
    system_init(system, "logger");
    events_map_import(event_map, system, event3);
    events_map_register_hook(event_map, system, logger_log, MAYBIFY(NULL), EVENT_LOG, MAYBIFY_FUNC(NULL));
    events_map_register_hook(event_map, system, test_hook, MAYBIFY(NULL), EVENT_LOG, MAYBIFY_FUNC(NULL));
}
