#include "entity.h"

#include "mem_wrap.h"
#include "component.h"
#include <stdint.h>


component_t * entity_add_component(entities_list_t *entities_list, entity_t *entity, uint32_t component_type_id) {
    component_t *component = component_new(&(entities_list->component_types[component_type_id]));
    list_insert_tail(&(entity->components), component);
    return component;
}

entity_t * entity_new (entities_list_t *entities_list, char *name) {
    entity_t *entity = mem_alloc(sizeof(*entity));
    entity->name = name;
    entity->id = entities_list->count;
    link_init(&(entity->entities_link));
    list_init(&(entity->components), component_t, entity_components_link);
    
    list_insert_tail(&(entities_list->entities), entity);
    entities_list->count += 1;
    
    return entity;
}

void entity_free(entity_t *entity) {
    mem_free(entity->name);
    link_remove_from_list(&(entity->entities_link));
    
    list_for_each(&(entity->components), component_t *, component) {
        component_free(component);
    }
    
    mem_free(entity);
}