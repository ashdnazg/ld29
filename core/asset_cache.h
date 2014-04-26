#ifndef __ASSET_CACHE_H__
#define __ASSET_CACHE_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "macros.h"
#include "int_list.h"

#define MAX_STR_CMP 255


typedef struct asset_node_s {
    link_t nodes_link;
    char *name;
    void *asset;
} asset_node_t;

typedef struct asset_cache_s {
    list_t nodes;
    MAYBE_FUNC(free_callback_t) free_cb;
} asset_cache_t;

void asset_cache_init(asset_cache_t *a_cache, MAYBE_FUNC(free_cb_t) free_cb);
    
asset_cache_t * asset_cache_new(MAYBE_FUNC(free_callback_t) free_cb);

void asset_cache_clean(asset_cache_t *a_cache);

void asset_cache_free(asset_cache_t *a_cache);

void asset_cache_add(asset_cache_t *a_cache, void *asset, const char *name);
MAYBE(void *) asset_cache_get(asset_cache_t *a_cache, const char *name);

#ifdef __cplusplus
}
#endif

#endif 
