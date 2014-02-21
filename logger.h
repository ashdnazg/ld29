#ifndef __LOGGER_H__
#define __LOGGER_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "system.h"



void init_logger(events_map_t *event_map, system_t *system);


#ifdef __cplusplus
}
#endif

#endif