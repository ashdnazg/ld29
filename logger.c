#include "logger.h"
#include "system.h"
#include "event.h"
#include "macros.h"
#include <stdlib.h>
#include <stdio.h>


void logger_log(events_list_t *events_list, system_t *system, MAYBE(void *) system_params, MAYBE(void *) sender_params) {
    if (UNMAYBE(sender_params) != NULL) {
        char *str = (char *) UNMAYBE(sender_params);
        printf("Log: %s\n", str);
    } else {
        printf("Log: Nothing\n");
    }
}

void init_logger(events_map_t *event_map, system_t *system) {
    system_init(system, "logger");
    events_map_register_hook(event_map, system, logger_log, MAYBIFY(NULL), EVENT_LOG, MAYBIFY_FUNC(NULL));
}
