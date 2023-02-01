// note: these files are provided by Ali Hatami, and are written by them.

/*
 * mm_alloc.h
 *
 * Exports a clone of the interface documented in "man 3 fuckmalloc".
 */

#pragma once

#ifndef _malloc_H_
#define _malloc_H_

 /* Define the block size since the sizeof will be wrong */
#define BLOCK_SIZE 40

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/**
 * @brief Allocates size bytes in the heap and returns the address
 * 
 * It uses first fit algorithm to find a place for new data. If it can't find
 * a space for new allocation, heap extension will occur and if that fails too
 * failure it is!
 * 
 * @param size the size of allocation
 * @return address of the begining of the allocated memory. it will
 *         return NULL if the size is 0 or on error.
 */

void define_min_max_allocation(size_t min, size_t max);

void* mm_malloc(size_t size);

/**
 * @brief reallocate the pointer with new memory size
 * 
 * If the size is smaller than the current size split will be done on the
 * block, but, if the size is bigger than the current size first it will check
 * if there is enough FREE space after this block. If it was available the
 * block would be expanded. If there wasn't enough space after program searches
 * for a FREE space generally and if any wasn't found heap will be expanded and
 * if that failed too nothing would change and NULL will be returned.
 * 
 * @param ptr previously allocated memory pointer
 * @param size new size that is needed
 * @return address of the new memory. NULL in case of failure
 */
void* mm_realloc(void* ptr, size_t size);

/**
 * @brief frees pre-allocated memory
 * 
 * given the pointer to a pre-allocated memory it will find the corresponding
 * block, make its FREE status true and fuse it if possible.
 * 
 * @param ptr pointer to a pre-allocated memory
 */
void mm_free(void* ptr);

typedef struct s_block *s_block_ptr;

/* block struct */
struct s_block {
    size_t size;
    struct s_block *next;
    struct s_block *prev;
    int is_free;
    void *ptr;
    /* A pointer to the allocated block */
    char data [0];
 };

/**
 * @brief Splits the block with size s and make another is_free part out of the
 *        other part.
 * 
 * If size of the block b is less than s nothing will be done but if the size
 * is bigger than s, there is some scenarios:
 * 
 *      1. there is another is_free part after this block, then the remaning will 
 *      be fused together and make a larger block. In this case the header will be
 *      moved to the is_free part and the size will be added to the current size
 *      of the next block.
 *
 *      2. size of the remaining part is bigger than the SIZE_BLOCK and
 *      metadata part can fit into it, so a new is_free part will be created and
 *      prev and next of the three blocks will be updated.
 * 
 *      3.1. remaining is less than BLOCK_SIZE but it is at the end of the 
 *      allocated memories so we will set break to the end of the segment that
 *      is needed and if the rest is needed will be allocated later.
 *
 *      3.2. size of the remaining part is not enough to create a new block so
 *      we assume that part is no man land and we won't tell user that!
 * 
 * @param b the block that will be splitted - should be a valid block
 * @param s size of the block AFTER reduction
 */
void split_block (s_block_ptr b, size_t s);


/**
 * @brief Try to fuse is_free blocks before and after of b together
 * 
 * it will fuse is_free adjacent blocks together. It only checks prev and next
 * blocks because if it be used after each mm_is_free then there only can't be
 * other sequence of is_free blocks (they where fused together when one of them
 * was is_freed)!
 * 
 * @param b the block to perform possible fusions on
 * @return pointer to the new b (the block that b was fused to) 
 */
s_block_ptr fusion(s_block_ptr b);

/**
 * @brief Get the block object corresponding to ptr
 * 
 * For sanity this function will iterate over all allocated blocks and retrive
 * the ptr of that. because ptr may be invalid and looking for BLOCK_SIZE bytes
 * ahead may cause conflicts and security issues.
 * 
 * @param p start of allocated memory
 * @return s_block_ptr 
 */
s_block_ptr get_block (void *p);

/**
 * @brief allocate memory to the end of the heap
 * 
 * This function will be used when there is not enough space in the allocated
 * memory. First it will check if there is is_free space at the end. if there were
 * one, it will allocate remaining amount of memory and add it to that block. 
 * If there wasn't a is_free block at the last a new block will be constructed. at
 * any time that allocation can't be done it will return NULL.
 * 
 * @param last the last element of the block lists.
 * @param s size to be allocated
 * @return NULL on failure and a pointer to the newly allocated(expanded) block
 */
s_block_ptr extend_heap (s_block_ptr last , size_t s);


#ifdef __cplusplus
}
#endif

#endif
