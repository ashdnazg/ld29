#include "macros.h"
#include "int_pheap.h"
#include "mem_wrap.h"
#include <assert.h>
#include <stdlib.h>

void _heap_init(heap_t *heap, unsigned int offset) {
    list_init(&(heap->root_list), heap_link_t, subheaps_link);
    heap->offset = offset;
}


heap_t * _heap_new(unsigned int offset) {
    heap_t *heap = mem_alloc(sizeof(heap_t));
    _heap_init(heap, offset);
    return heap;
}

void _heap_link_init(heap_link_t *heap_link, unsigned int offset) {
    list_init(&(heap_link->subheaps), heap_link_t, subheaps_link);
    link_init(&(heap_link->subheaps_link));
    heap_link->key = 0;
}

bool heap_is_empty(heap_t *heap) {
    return list_is_empty(&(heap->root_list));
}

void * heap_get_node(heap_t *heap, heap_link_t *heap_link) {
    return (void *) ((size_t) heap_link - heap->offset);
}

void * heap_get_min(heap_t *heap) {
    if (heap_is_empty(heap)) {
        return NULL;
    }
    assert(list_head(&(heap->root_list)) == list_tail(&(heap->root_list)));
    return heap_get_node(heap, list_head(&(heap->root_list)));
}

heap_link_t * heap_get_link(heap_t *heap, void *node) {
    return (heap_link_t *) ((size_t) node + heap->offset);
}

void heap_insert(heap_t *heap, void *node, unsigned int key) {
    heap_link_t *heap_link = heap_get_link(heap, node);
    
    heap_link->key = key;
    list_insert_head(&(heap->root_list), heap_link);
    heap_link_merge(heap_link, link_next(&(heap_link->subheaps_link)));
}

void * heap_delete_min(heap_t *heap) {
    heap_link_t *heap_link_root = NULL;
    void *old_root = NULL;
    
    if (heap_is_empty(heap)) {
        return NULL;
    }
    
    heap_link_root = list_head(&(heap->root_list));
    old_root = heap_get_min(heap);
    heap_link_unlink(heap_link_root);
    return old_root;
}


heap_link_t * heap_link_merge(heap_link_t *heap_link_a, heap_link_t *heap_link_b) {
    if (heap_link_a == NULL) {
        return heap_link_b;
    }
    if (heap_link_b == NULL) {
        return heap_link_a;
    }
    
    if (heap_link_a->key < heap_link_b->key) {
        list_insert_tail(&(heap_link_a->subheaps), heap_link_b);
        return heap_link_a;
    } else {
        list_insert_tail(&(heap_link_b->subheaps), heap_link_a);
        return heap_link_b;
    }
}

void heap_link_unlink(heap_link_t *heap_link) {
    heap_link_t * heap_link_merged = heap_link_merge_subheaps(heap_link);
    if (heap_link_merged != NULL) {
        assert(link_is_linked(&(heap_link->subheaps_link)));
        link_insert_before(&(heap_link_merged->subheaps_link), heap_link_merged, &(heap_link->subheaps_link));
    }
    link_unlink(&(heap_link->subheaps_link));
}


void heap_delete(heap_t *heap, void *node) {
    heap_link_t *heap_link = heap_get_link(heap, node);
    heap_link_unlink(heap_link);
}

heap_link_t * heap_link_merge_subheaps(heap_link_t *heap_link) {
    heap_link_t *heap_link_a = NULL;
    heap_link_t *heap_link_b = NULL;
    list_t *subheaps = NULL;
    subheaps = &(heap_link->subheaps);
    if (list_is_empty(subheaps)) {
        return NULL;
    }
    
    heap_link_a = list_head(subheaps);
    for (;;) {        
        heap_link_b = list_next(subheaps, heap_link_a);
        if (heap_link_b == NULL) {
            break;
        }
        heap_link_a = list_next(subheaps, heap_link_merge(heap_link_a, heap_link_b));
        if (heap_link_a == NULL) {
            break;
        }
    }
    
    heap_link_a = list_tail(subheaps);
    for (;;) {
        heap_link_b = list_prev(subheaps, heap_link_a);
        if (heap_link_b == NULL) {
            break;
        }
        heap_link_a = heap_link_merge(heap_link_a, heap_link_b);
    }
    return heap_link_a;
}
