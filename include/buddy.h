// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#ifndef _buddy_H_
#define _buddy_H_

#ifdef __cplusplus
extern "C" {
#endif


#define BLOCK_SIZE 48

#include <stdlib.h>

/**
 * @brief allocates size bytes in the memory
 * 
 * 
 * @param size size to be allocated
 * @return void* 
 */
void* bud_malloc(size_t size);

/**
 * @brief reallocate the pointer with new memory size
 * 
 * @param ptr previously allocated memory pointer
 * @param size new size that is needed
 * @return address of the new memory. NULL in case of failure
 */
void* bud_realloc(void* ptr, size_t size);

/**
 * @brief frees pre-allocated memory
 * 
 * @param ptr pointer to a pre-allocated memory
 */
void bud_free(void* ptr);


/**
 * @brief Shows the status of the allocated memory
 * 
 * Allocated blocks
 * Free blocks
 * 
 */
void bud_show_stats();

typedef struct bud_block *bud_block_ptr;

/* block struct */
struct bud_block {
    size_t size;
    struct bud_block *next;
    struct bud_block *prev;
    int is_free;
    int depth;
    int is_left;
    void *ptr;
    /* A pointer to the allocated block */
    char data [0];
 };

#ifdef __cplusplus
}
#endif

#endif