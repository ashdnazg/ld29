#ifndef __EVENT_TYPES_H__
#define __EVENT_TYPES_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum event_type_e {
    EVENT_NEW_STEP,
    EVENT_NEW_FRAME,
    
    EVENT_COUNT,
} event_type_t;

typedef struct custom_event_type_s {
    unsigned int id;
    const char *name;
} custom_event_type_t;

#ifdef __cplusplus
}
#endif

#endif