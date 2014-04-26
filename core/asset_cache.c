#include "macros.h"
#include "asset_cache.h"
#include "mem_wrap.h"
#include "int_list.h"
#include <string.h>
#include <stdio.h>

asset_node_t * asset_node_new(void *asset, const char *name) {
    asset_node_t *asset_node = mem_alloc(sizeof(asset_node_t));
    asset_node->name = mem_strdup(name);
    asset_node->asset = asset;
    link_init(&(asset_node->nodes_link));
    return asset_node;
}

void asset_node_free(asset_node_t *asset_node) {
    link_remove_from_list(&(asset_node->nodes_link));
    mem_free(asset_node->name);
    mem_free(asset_node);
}


void asset_cache_init(asset_cache_t *a_cache, MAYBE_FUNC(free_cb_t) free_cb) {
    list_init(&(a_cache->nodes), asset_node_t, nodes_link);
    a_cache->free_cb = free_cb;
}

asset_cache_t * asset_cache_new(MAYBE_FUNC(free_cb_t) free_cb) {
    asset_cache_t *a_cache = mem_alloc(sizeof(asset_cache_t));
    asset_cache_init(a_cache, free_cb);
    return a_cache;
}

void asset_cache_clean(asset_cache_t *a_cache) {
    if (UNMAYBE(a_cache->free_cb) != NULL) {
        list_for_each(&(a_cache->nodes), asset_node_t *, asset_node) {
            ((free_callback_t) UNMAYBE(a_cache->free_cb))(asset_node->asset);
        }
    }
    list_for_each(&(a_cache->nodes), asset_node_t *, asset_node) {
        asset_node_free(asset_node);
    }
}

void asset_cache_free(asset_cache_t *a_cache) {
    asset_cache_clean(a_cache);
    mem_free(a_cache);
}



void asset_cache_add(asset_cache_t *a_cache, void *asset, const char *name) {
    asset_node_t *asset_node = asset_node_new(asset, name);
    list_insert_tail(&(a_cache->nodes), asset_node);
}


MAYBE(void *) asset_cache_get(asset_cache_t *a_cache, const char *name) {
    list_for_each(&(a_cache->nodes), asset_node_t *, asset_node){
        if(strncmp(name, asset_node->name, MAX_STR_CMP) == 0){
            return MAYBIFY(asset_node->asset);
        }
    }
    return MAYBIFY(NULL);
}
