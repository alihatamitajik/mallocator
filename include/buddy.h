// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#ifndef _buddy_H_
#define _buddy_H_

#ifdef __cplusplus
extern "C" {
#endif


#define BUD_BLOCK_SIZE 48

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @brief allocates size bytes in the memory
 * 
 * Allocation Process:
 *  - find the best fit (equal or smallest possible) in the free allocated list
 *  - if found an equal allocate it
 *  - if found a bigger free block split it until fit.
 *  - if 404 not found, then allocate twice as allocated size now and do the
 *    splitting.
 *  - if can't allocate new storage, return NULL.
 * 
 * After allocation it will set all bytes to `fill` value. Note that fill is
 * passed as integer but it will use unsigned char conversion of it.
 * 
 *  NOTE: it will return NULL on impossible requests (irrational or not enough
 *        storage or not in bound)
 * 
 * @param size size to be allocated
 * @param fill fills allocated size with fill value
 * @return void* 
 */
void* bud_malloc(size_t size, int fill);

/**
 * @brief reallocate the pointer with new memory size
 * 
 * this function is combination of these three jobs:
 *  - malloc
 *  - memcpy
 *  - free
 * 
 * NOTE:
 *  - if size is not positive, then it is equivalent of free
 *  - if ptr is NULL, then it is equivalent of malloc
 *  - if ptr was not allocated before it will return NULL
 *  - if request size (next_pow2) is same as previous one, it will return ptr
 *  - if allocation of size failed, NULL is returned
 * 
 * @param ptr previously allocated memory pointer
 * @param size new size that is needed
 * @param fill fills allocated size with fill value
 * @return address of the new memory. NULL in case of failure
 */
void* bud_realloc(void* ptr, size_t size, int fill);

/**
 * @brief frees pre-allocated memory
 * 
 * This functions marks the pointer free and tries to coalesce as much as
 * possible with childs in the same depth, same parent and rightness of the
 * next block be 1 and current zero (In other words we test the block to
 * be the left child and coalesce it with the right child it both are free).
 * If the coalesce process was successful, then it will try for another time
 * until it cannot anymore.
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

/**
 * @brief sets minimum size that can be allocated
 * 
 * NOTES: 
 *      - if min provided is bigger than the max, it will be ignored.
 *      - if min provided in less than 0, 0 will be the min.
 * 
 * @param min min value
 * @return int setted min value
 */
int bud_set_minimum(int min);

/**
 * @brief sets maximum size that can be allocated
 * 
 * NOTES: 
 *      - if max provided is smaller than the min, it will be ignored.
 *      - if max provided is -1, no maximum checking is done.
 *      - max should be at least 1 or -1 for no limit
 * 
 * @param max max value
 * @return int setted max value
 */
int bud_set_maximum(int max);

typedef struct bud_block *bud_meta;

/**
 * @brief this struct will be used as metadata header
 * 
 * size is the order of the block allocated (i.e. 64, 128, ...) and not
 * the size user requested.
 * 
 * next and prev are used for linked list structure of the memory.
 * 
 * is_free is freeness of the block which will be used as a marker so we can 
 * coalesce free memories if possible.
 * 
 * depth is how many times split was done on the memory.
 * 
 * rightness is a number which ith bit of it will show weather the parent was
 * right node or left node of the tree. It will be used to compare left and
 * right nodes to coalesce two blocks with same parent.
 * 
 * ptr is the pointer returned to the user and is kept for simplicity of search
 * process.
 * 
 * data is dummy pointer (no bytes is occupied to the first data block of the
 * memory).
 */
struct bud_block {
    size_t size;
    struct bud_block *next;
    struct bud_block *prev;
    int is_free;
    int depth;
    int rightness;
    void *ptr;
    /* A pointer to the allocated block */
    char data [0];
 };

#ifdef __cplusplus
}
#endif

#endif