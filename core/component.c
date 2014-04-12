#include "mem_wrap.h"
#include "component.h"

component_t * component_new(component_type_t *type) {
    component_t * component = mem_alloc(sizeof(*component) + type->data_size);
    void *data = (void *) (component + 1);
    component->type = type;
    link_init(&(component->entity_components_link));
    link_init(&(component->type_components_link));
    type->init_callback(data);
    
    return component;
}

void component_free(component_t *component) {
    void *data = (void *) (component + 1);
    link_remove_from_list(&(component->entity_components_link));
    link_remove_from_list(&(component->type_components_link));
    component->type->clean_callback(data);
    mem_free(component);
}