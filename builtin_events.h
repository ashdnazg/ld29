#ifndef __BUILTIN_EVENTS_H__
#define __BUILTIN_EVENTS_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum builtin_events_e {
    EVENT_NEW_STEP,
    EVENT_NEW_FRAME,
    EVENT_EXIT,
    EVENT_PAUSE,
    EVENT_LOG,
    
    EVENTS_COUNT,
};

typedef struct custom_event_type_s {
    unsigned int id;
    const char *name;
} custom_event_type_t;

#ifdef __cplusplus
}
#endif

#endif
