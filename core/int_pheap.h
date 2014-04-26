#ifndef __INT_PHEAP_H__
#define __INT_PHEAP_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include "macros.h"
#include "int_list.h"

typedef struct heap_s {
    list_t root_list; //Holds either 0 or 1 heap_link_t's.
    unsigned int offset;
} heap_t;

typedef struct heap_link_s {
    list_t subheaps;
    link_t subheaps_link;
    unsigned int key;
} heap_link_t;


#define heap_init(heap, node_type, heap_link_name) _heap_init(heap, (unsigned int) offsetof(node_type, heap_link_name))
void _heap_init(heap_t *heap, unsigned int offset);

#define heap_new(node_type, heap_link_name) _heap_new(offsetof(node_type, heap_link_name));
heap_t * _heap_new(unsigned int offset);

bool heap_is_empty(heap_t *heap);

void * heap_get_min(heap_t *heap);
void * heap_delete_min(heap_t *heap);

heap_link_t * heap_get_link(heap_t *heap, void *node);
void heap_insert(heap_t *heap, void *node, unsigned int key);
void heap_delete(heap_t *heap, void *node);

#define heap_link_init(node, heap_link_name) _heap_link_init(&((node)->heap_link_name) , (unsigned int) ((size_t) (node) - ((size_t) &((node)->heap_link_name))))

void _heap_link_init(heap_link_t *heap_link, unsigned int offset);
heap_link_t * heap_link_merge(heap_link_t *heap_link_a, heap_link_t *heap_link_b);
heap_link_t * heap_link_merge_subheaps(heap_link_t *heap_link);
void heap_link_unlink(heap_link_t *heap_link);
#ifdef __cplusplus
}
#endif

#endif
