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


component_t * entity_add_component(entities_list_t *entities_list, entity_t *entity, uint32_t component_type_id) {
    component_t *component = component_new(&(entities_list->components_map), component_type_id);
    list_insert_tail(&(entity->components), component);
    return component;
}

entity_t * entity_new(entities_list_t *entities_list, char *name) {
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