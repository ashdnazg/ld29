#ifndef __COMPONENT_H__
#define __COMPONENT_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"
#include "int_list.h"
#include <stdint.h>


typedef struct component_header_s component_t;
typedef struct component_header_s component_header_t;
typedef struct components_map_s components_map_t;

typedef void (*data_init_t)(void *);
typedef void (*data_clean_t)(void *);

typedef struct component_type_s component_type_t;

struct components_map_s {
    bool initialized;
    size_t count;
    list_t pending_component_imports;
    list_t pending_component_exports;
    component_type_t * component_types;
};

#include "entity.h"
#include "system.h"

#define CUSTOM_COMPONENT(var) __CUSTOM_COMPONENT_ ## var
#define LOCAL_COMPONENT(system, name) ((system)->local_components_map[CUSTOM_COMPONENT(name)])
#define LOCAL_COMPONENTS enum __LOCAL_COMPONENT {
#define END_LOCAL_COMPONENTS ,__LOCAL_COMPONENTS_COUNT};

#define COMPONENT_DATA(name) struct __ ## name ## _data_s

#define INIT_DATA(name) __ ## name ## _data_init

#define CLEAN_DATA(name) __ ## name ## _data_clean

#define COMPONENT_NAME_MAX_LENGTH 100


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

typedef struct pending_component_import_s {
    link_t pending_component_imports_link;
    system_t *system;
    const char *name;
    uint32_t local_index;
} pending_component_import_t;

typedef struct pending_component_export_s {
    link_t pending_component_exports_link;
    const char *name;
    data_init_t init_callback;
    data_clean_t clean_callback;
    uint32_t data_size;
} pending_component_export_t;


void components_map_init(components_map_t *components_map);

void components_map_process_pending(components_map_t *components_map);

void components_map_clean(components_map_t *components_map);

component_t * component_new(components_map_t *components_map, uint32_t type_id);

void component_free(component_t *component);

#define components_map_export(map, system, name) \
    do { \
        system_init_local_components_map(system, __LOCAL_COMPONENTS_COUNT); \
        _components_map_export(map, #name, INIT_DATA(name), CLEAN_DATA(name), sizeof(COMPONENT_DATA(name)));\
        _components_map_import(map, system, #name, CUSTOM_COMPONENT(name));\
    } while (0);

void _components_map_export(components_map_t *components_map, const char *name, data_init_t init_callback, data_clean_t clean_callback, uint32_t data_size);

#define components_map_import(map, system, name) \
    do { \
        system_init_local_components_map(system, __LOCAL_COMPONENTS_COUNT); \
        _components_map_import(map, system, #name, CUSTOM_COMPONENT(name));\
    } while (0);

void _components_map_import(components_map_t *components_map, system_t *system, const char *name, uint32_t local_index);


#ifdef __cplusplus
}
#endif

#endif
