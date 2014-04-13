#include "core/system.h"
#include "core/macros.h"
#include "core/component.h"
#include "core/entity.h"
#include "core/game.h"
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
    system_t *sys = system_new();
    entity_t *ent;
    game_t game;
    //system_t logger;
    game_init(&game);
    sys->name = "system1";
    //init_logger(&map, &logger);
    game_export_component(&game, sys, component1);
    game_export_component(&game, sys, component2);
    game_start(&game);
    printf("processed\n");
    ent = entity_create(&game, mem_strdup("test entity"));
    entity_add_component(&game, ent, LOCAL_COMPONENT(sys,component1));
    entity_add_component(&game, ent, LOCAL_COMPONENT(sys,component2));
    
    
    game_clean(&game);
    printf("entities list cleaned\n");
    
    system_free(sys);
    //system_clean(&logger);
    printf("system_cleaned\n");

    mem_wrap_print_mallocs();
    return 0;
}
