#include "core/system.h"
#include "core/macros.h"
#include "core/component.h"
#include "core/entity.h"
#include "core/mem_wrap.h"
#include <stdlib.h>
#include <stdio.h>


LOCAL_COMPONENTS
    CUSTOM_COMPONENT(component1),
    CUSTOM_COMPONENT(component2)
END_LOCAL_COMPONENTS

COMPONENT_DATA(component1) {
    int id;
};

void INIT_DATA(component1) (void *data) {
    COMPONENT_DATA(component1) *d = (COMPONENT_DATA(component1) *) data;
    d->id = 1;
}

void CLEAN_DATA(component1) (void *data) {
    COMPONENT_DATA(component1) *d = (COMPONENT_DATA(component1) *) data;
    printf("cleaned: %d\n", d->id);
}

COMPONENT_DATA(component2) {
    int ido;
};

void INIT_DATA(component2) (void *data) {
    COMPONENT_DATA(component2) *d = (COMPONENT_DATA(component2) *) data;
    d->ido = 2;
}

void CLEAN_DATA(component2) (void *data) {
    COMPONENT_DATA(component2) *d = (COMPONENT_DATA(component2) *) data;
    printf("cleaned: %d\n", d->ido);
}


int main(int argc, char* argv[]) {
    mem_wrap_init();
    system_t sys;
    entity_t *ent;
    components_map_t *map;
    //system_t logger;
    entities_list_t e_list;
    entities_list_init(&e_list);
    map = &(e_list.components_map);
    system_init(&sys, "system");
    //init_logger(&map, &logger);
    components_map_export(map, &sys, component1);
    components_map_export(map, &sys, component2);
    components_map_process_pending(map);
    printf("processed\n");
    ent = entity_new(&e_list, mem_strdup("test entity"));
    entity_add_component(&e_list, ent, LOCAL_COMPONENT(&sys,component1));
    entity_add_component(&e_list, ent, LOCAL_COMPONENT(&sys,component2));
    
    
    entities_list_clean(&e_list);
    printf("entities list cleaned\n");
    
    system_clean(&sys);
    //system_clean(&logger);
    printf("system_cleaned\n");

    mem_wrap_print_mallocs();
    return 0;
}
