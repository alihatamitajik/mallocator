/**
 * @file myalloc.h
 * @brief Allocation Library
 * @version 0.1
 * @date 2023-02-03
 * 
 * A simple allocation library which has two method of allocation:
 *      1. First Fit 
 *      2. Buddy
 * 
 * + A minimum and maximum limit can be set for allocations
 * + First fit uses 40B and Buddy uses 48B of allocations as metadata.
 * 
 * 
 * Time complexities:
 *      All Operations are O(N) in time complexity.
 * 
 * Fragmentation:
 *      In worst case, both algorithms can waste ~50% of the memory with
 *      internal fragmentation (But some algorithms are implemented to reduce
 *      this effect).
 * 
 * NOTE: if you does not set the algorithm before the first call of the methods,
 * First fit will be considered as your algorithm and cannot be changed further
 * in your code.
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#pragma once

#ifndef _myalloc_H_
#define _myalloc_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "buddy.h"
#include "firstfit.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define ALG_CHECK if (!alg.is_defined) \
    { \
        alg.is_defined = 1; \
    }

struct AlgorithmWrapper
{
    int is_defined;
    void* (*my_malloc)(size_t, int);
    void* (*my_realloc)(void*, size_t, int);
    void  (*my_free)(void*);
    void (*show_stats)();
    int   (*set_maximum)(int);
    int   (*set_minimum)(int);
} alg = {0, 
    &ff_malloc, 
    &ff_realloc, 
    &ff_free,
    NULL, 
    &ff_set_maximum,
    &ff_set_minimum
};


/**
 * @brief Set the algorithm
 * 
 * `algorithm` can be one of "firstfit" or "buddy". This function should be
 * used before any use of other function, otherwise, first fit will be
 * considered as the allocation algorithm. 
 * 
 * ERRORS: errno will be
 *  31: if defined before
 *  22: if algorithm does not match "firstfit" nor "buddy"
 * 
 * @param algorithm 
 * @return int -1 if not set, 1 if firstfit, 2 if buddy is set.
 */
int set_algorithm(const char *algorithm)
{
    if (alg.is_defined) {
        errno = EMLINK;
        return -1;
    }
    
    if (strcasecmp(algorithm, "firstfit") == 0)
    {
        alg.is_defined = 1;
        return 1;
    } else if (strcasecmp(algorithm, "buddy") == 0)
    {
        alg = {
            1, 
            &bud_malloc, 
            &bud_realloc, 
            &bud_free, 
            &bud_show_stats, 
            &bud_set_maximum, 
            &bud_set_minimum
        };
        return 2;
    } else {
        errno = EINVAL;
        return -1;
    }
}

/**
 * @brief Allocates `size` bytes and set every byte with `fill`
 * 
 * @see bud_malloc
 * @see ff_malloc
 * 
 * @param size size of allocation
 * @param fill filling byte
 * @return void* NULL if allocation failed or pointer to the allocated space
 */
void* my_malloc(size_t size, int fill)
{
    ALG_CHECK;
    return (*alg.my_malloc)(size, fill);
}

void* my_realloc(void* ptr, size_t size, int fill)
{
    ALG_CHECK;
    return (*alg.my_realloc)(ptr, size, fill);
}

void my_free(void* ptr)
{
    ALG_CHECK;
    (*alg.my_free)(ptr);
}

void show_stats()
{
    ALG_CHECK;
    (*alg.show_stats)();
}

int set_maximum(int value)
{
    ALG_CHECK;
    return (*alg.set_maximum)(value);
}

int set_minimum(int value)
{
    ALG_CHECK;
    return (*alg.set_minimum)(value);
}

#ifdef __cplusplus
}
#endif

#endif // _myalloc_H_ guard