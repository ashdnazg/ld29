#include "entity.h"

#include "mem_wrap.h"
#include "component.h"
#include <stdint.h>

void entities_list_init(entities_list_t *entities_list) {
    entities_list->count = 0;
    list_init(&(entities_list->entities), entity_t, entities_link);
    components_map_init(&(entities_list->components_map));
}

void entities_list_clean(entities_list_t *entities_list) {
    list_for_each(&(entities_list->entities), entity_t *, entity) {
        entity_free(entity);
    }
    components_map_clean(&(entities_list->components_map));
}

void entity_free(entity_t *entity) {
    mem_free(entity->name);
    link_remove_from_list(&(entity->entities_link));
    
    list_for_each(&(entity->components), component_t *, component) {
        component_free(component);
    }
    
    mem_free(entity);
}

entity_t * entity_new(const char *name, uint32_t id) {
    entity_t *entity = mem_alloc(sizeof(*entity));
    entity->name = mem_strdup(name);
    entity->id = id;
    link_init(&(entity->entities_link));
    list_init(&(entity->components), component_t, entity_components_link);
    
    return entity;
}
