#ifndef __ENTITY_H__
#define __ENTITY_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"

typedef struct entity_s entity_t;
typedef struct entities_list_s entities_list_t;
#include "int_list.h"
#include "component.h"

struct entities_list_s {
    uint32_t count;
    list_t entities;
    components_map_t components_map;
};

struct entity_s {
    char *name;
    uint32_t id;
    link_t entities_link;
    list_t components;
};


void entities_list_init(entities_list_t *entities_list);
void entities_list_clean(entities_list_t *entities_list);

component_t * entity_add_component(entities_list_t *entities_list, entity_t *entity, uint32_t component_type_id);

entity_t * entity_new (entities_list_t *entities_list, char *name);

void entity_free(entity_t *entity);

#ifdef __cplusplus
}
#endif

#endif
