#include "macros.h"
#include "mem_wrap.h"
#include "component.h"
#include <string.h>
#include <stdio.h>


void components_map_init(components_map_t *components_map) {
    components_map->initialized = FALSE;
    components_map->count = 0;
    list_init(&(components_map->pending_component_imports), pending_component_import_t, pending_component_imports_link);
    list_init(&(components_map->pending_component_exports), pending_component_export_t, pending_component_exports_link);
    components_map->component_types = NULL;
}

component_t * component_new(components_map_t *components_map, uint32_t type_id) {
    component_type_t *type = &(components_map->component_types[type_id]);
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

void _components_map_export(components_map_t *components_map, const char *name, data_init_t init_callback, data_clean_t clean_callback, uint32_t data_size) {
    pending_component_export_t *pending_component_export = mem_alloc(sizeof(*pending_component_export));
    link_init(&(pending_component_export->pending_component_exports_link));
    pending_component_export->name = name;
    pending_component_export->init_callback = init_callback;
    pending_component_export->clean_callback = clean_callback;
    pending_component_export->data_size = data_size;
    
    list_insert_tail(&(components_map->pending_component_exports), pending_component_export);
    ++(components_map->count);
}


void _components_map_import(components_map_t *components_map, system_t *system, const char *name, uint32_t local_index) {
    pending_component_import_t *pending_component_import = mem_alloc(sizeof(*pending_component_import));
    link_init(&(pending_component_import->pending_component_imports_link));
    pending_component_import->name = name;
    pending_component_import->system = system;
    pending_component_import->local_index = local_index & 0x3FFFFFFF;
    
    list_insert_tail(&(components_map->pending_component_imports), pending_component_import);
}

bool compare_pending_component_exports(pending_component_export_t *pending_component_export_a, pending_component_export_t *pending_component_export_b) {
    return strncmp(pending_component_export_a->name, pending_component_export_b->name, COMPONENT_NAME_MAX_LENGTH) < 0;
}

bool compare_pending_component_imports(pending_component_import_t *pending_component_import_a, pending_component_import_t *pending_component_import_b) {
    return strncmp(pending_component_import_a->name, pending_component_import_b->name, COMPONENT_NAME_MAX_LENGTH) < 0;
}

void pending_component_import_free(pending_component_import_t *pending_component_import) {
    link_remove_from_list(&(pending_component_import->pending_component_imports_link));
    mem_free(pending_component_import);
}

void pending_component_export_free(pending_component_export_t *pending_component_export) {
    link_remove_from_list(&(pending_component_export->pending_component_exports_link));
    mem_free(pending_component_export);
}

void components_map_process_pending(components_map_t *components_map) {
    int i;
    int cmp;
    components_map->component_types = mem_alloc(sizeof(*(components_map->component_types)) * components_map->count);
    
    
    list_sort(&(components_map->pending_component_imports), (cmp_cb_t) compare_pending_component_imports);
    list_sort(&(components_map->pending_component_exports), (cmp_cb_t) compare_pending_component_exports);
    i = 0;
    list_for_each(&(components_map->pending_component_exports), pending_component_export_t *, pending_component_export) {
        if (next_pending_component_export != NULL && compare_pending_component_exports(next_pending_component_export, pending_component_export)) {
            printf("Two components with the name: %s\n", pending_component_export->name);
            //ERROR: two components with the same name
        }
        list_for_each(&(components_map->pending_component_imports), pending_component_import_t *, pending_component_import) {
            cmp = strncmp(pending_component_import->name, pending_component_export->name, COMPONENT_NAME_MAX_LENGTH);
            if (cmp == 0) {
                pending_component_import->system->local_components_map[pending_component_import->local_index] = i;
                //printf("Imported: %s mapped: %d -> %d\n", pending_component_import->name, pending_component_import->local_index, i);
                pending_component_import_free(pending_component_import);
            } else if (cmp < 0) {
                printf("Imported component not found: %s\n", pending_component_import->name);
                //ERROR: couldn't import an component
            }
            
        }
        components_map->component_types[i].data_size = pending_component_export->data_size;
        list_init(&(components_map->component_types[i].components), component_t, type_components_link);
        components_map->component_types[i].init_callback = pending_component_export->init_callback;
        components_map->component_types[i].clean_callback = pending_component_export->clean_callback;
        pending_component_export_free(pending_component_export);
        ++i;
    }
    components_map->initialized = TRUE;
}

void components_map_clean(components_map_t *components_map) {
    int i = 0;
    
    if (NULL != components_map->component_types) {
        for (i = 0; i < components_map->count; ++i) {
            list_for_each(&(components_map->component_types[i].components), component_t *, component) {
                component_free(component);
            }
        }
        mem_free(components_map->component_types);
    }
    list_for_each(&(components_map->pending_component_imports), pending_component_import_t *, pending_component_import) {
        pending_component_import_free(pending_component_import);
    }
    list_for_each(&(components_map->pending_component_exports), pending_component_export_t *, pending_component_export) {
        pending_component_export_free(pending_component_export);
    }
}
