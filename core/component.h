#ifndef __COMPONENT_H__
#define __COMPONENT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"

typedef struct component_header_s component_t;
typedef struct component_header_s component_header_t;
typedef void (*data_init_t)(void *);
typedef void (*data_clean_t)(void *);

typedef struct component_type_s component_type_t;

#include <stdint.h>
#include "int_list.h"
#include "entity.h"
#include "system.h"


struct component_type_s {
    uint32_t data_size;
    list_t components;
    data_init_t init_callback;
    data_clean_t clean_callback;
};

struct component_header_s {
    component_type_t *type;
    link_t entity_components_link;
    link_t type_components_link;
    system_t *system;
    entity_t *entity_id;
};


component_t * component_new(component_type_t *component_type);

void component_free(component_t *component);

#ifdef __cplusplus
}
#endif

#endif
