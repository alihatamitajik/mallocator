// note: these files are provided by Ali Hatami, and are written by them.

/*
 * firstfit.h
 *
 * Exports a clone of the interface documented in "man 3 fuckmalloc".
 */

#pragma once

#ifndef _firstfit_H_
#define _firstfit_H_

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
 * @param fill fills allocated size with fill value
 * @return address of the begining of the allocated memory. it will
 *         return NULL if the size is 0 or on error.
 */
void* ff_malloc(size_t size, int fill);

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
 * @param fill fills allocated size with fill value
 * @return address of the new memory. NULL in case of failure
 */
void* ff_realloc(void* ptr, size_t size, int fill);

/**
 * @brief frees pre-allocated memory
 * 
 * given the pointer to a pre-allocated memory it will find the corresponding
 * block, make its FREE status true and fuse it if possible.
 * 
 * @param ptr pointer to a pre-allocated memory
 */
void ff_free(void* ptr);

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
int ff_set_minimum(int min);

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
int ff_set_maximum(int max);


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

 void ff_show_stats();

#ifdef __cplusplus
}
#endif

#endif
