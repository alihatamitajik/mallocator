// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "buddy.h"

#define MIN(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

#define MAX(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

bud_meta head = NULL;

/** Initial Min limit (no limit) */
size_t min_limit = 0;

/** initial Max limit (no limit) */
size_t max_limit = -1;

size_t sum_allocated = 0;


/**
 * @brief return the smallest power of two value greater than x ([1], p 48)
 * 
 * Other methods was possible for this work like using A32 builtin bit scan 
 * revers (bsr) instruction or using mathematical approach with logarithms and
 * ceiling.
 * 
 * [1] Baxter, Michael. "hacker's delight." Linux Journal 2003.108 (2003).
 *
 */
static __attribute__((noinline)) unsigned next_pow2(unsigned x)
{
	x -= 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	
	return x + 1;
}


/**
 * @brief Splits the block into half
 * 
 * 
 * @param b the block that will be splitted - should be a valid block
 */
void split (bud_meta b)
{

}


/**
 * @brief Try to coalesce is_free blocks as much as possible
 * 
 * This function use an iterative behavior (i.e. it coalesce blocks if it was
 * successful it tries to coalesce blocks another time until it cant anymore).
 * 
 * This way we make sure that all possible combination of coalesce happened,
 * although this method is compute intensive and better implementations was
 * possible.
 * 
 * @return int 1 if a coalesce occurred and 0 if not. 
 */
int coalesce()
{
    return 0;
}

/**
 * @brief Get the block object corresponding to ptr
 * 
 * 
 * @param p start of allocated memory
 * @return s_block_ptr 
 */
bud_meta get_block (void *p)
{
    return NULL;
}

/**
 * @brief double the size of the heap allocated by now
 * 
 * This functions double the memory we had allocated previously (sum_allocated)
 * and update the depth of the depth of previous data.
 * 
 * NOTE: each time we extend the heap, depth of all blocks should be increased
 * by one.
 * 
 * NOTE: since we are adding the newly block to the right, we don't need to
 * update the rightness of the metadata.
 * 
 * @param last the last element of the block lists.
 * @param s size to be allocated
 * @return NULL on failure and a pointer to the newly allocated(expanded) block
 */
bud_meta extend_heap ()
{
    void* mem;
    mem = (void *) sbrk(sum_allocated);
    if (mem == (void *) -1)
    {
        return NULL;
    }

    bud_meta header = (bud_meta) mem;

    // increment the depth of previous
    bud_meta prev, block = head;
    while (block)
    {
        block->depth += 1;
        prev = block;
        block = block->next;
    }

    // define the new block header
    header->size = sum_allocated;
    header->is_free = 1;
    header->rightness = 1;
    header->depth = 1;
    header->ptr = &header->data;

    // adjust the links
    header->next = NULL;
    header->prev = prev;
    prev->next = header;

    sum_allocated <<= 1;

    return header;
}

/**
 * @brief Initializes the heap by size given to it
 * 
 * @param s size of allocation
 * @return bud_meta 
 */
bud_meta init_heap(size_t size)
{
    void* mem;
    mem = (void *) sbrk(size);
    
    if (mem == (void*) -1)
    {
        return NULL; // couldn't allocate
    } 
    
    bud_meta header = (bud_meta) mem;
    header->size = size;
    header->is_free = 1;
    header->next = NULL;
    header->prev = NULL;
    header->depth = 0;
    header->rightness = 0;
    header->ptr = &header->data;
    head = header;

    sum_allocated = size;

    return header;
}


/**
 * @brief Get the smallest fit of the data or find smallest to split
 * 
 * @param size 
 * @return bud_meta 
 */
bud_meta get_best_fit(size_t size)
{
    bud_meta block = head;
    bud_meta best = NULL;
    while (block)
    {
        if (block->is_free)
        {
            if (block->size == size)
            {
                return block;
            }
            else if (block->size > size && (best == NULL || best->size > block->size))
            {
                best = block;
            }
        }
        
        block = block->next;
    }

    return best;
}

bud_meta shrink_to_size(bud_meta bm, size_t size)
{
    if (bm->size < size) {
        return NULL;
    } else {
        while (bm->size > size)
        {
            split(bm);
        }
        return bm;
    }
}

/**
 * @brief Get the block pointer
 * 
 * for more information see the bud_malloc documentation
 * 
 * @param size size to get
 * @return bud_meta NULL if couldn't else a pointer to header of *free* data
 */
bud_meta alloc_block(size_t size)
{
    
    if (head == NULL)
    { // if no allocation before this extend the heap
        return init_heap(size);
    } 
    else 
    { /* find the best fit block and if not found extend the heap
         extension is done until it */
        bud_meta best_fit = get_best_fit(size);
        if (best_fit != NULL)
        {
            return shrink_to_size(best_fit, size);
        } else { // no fitted block found, we should extend the heap
            bud_meta last_extended;
            do
            {
                last_extended = extend_heap();
                if (last_extended == NULL)
                    return NULL;
            } while (sum_allocated/2 < size);
            // shrink the new allocated heap to the size we wanted
            return shrink_to_size(last_extended, size);
        }
    }
}

void* bud_malloc(size_t size)
{
    // if it violates the boundaries
    if (size < min_limit || (max_limit != -1 && size > max_limit))
    {
        return NULL;
    }
    bud_meta bbp = alloc_block(next_pow2(BLOCK_SIZE + size));
    if (bbp == NULL) 
    {
        return NULL;
    } else {
        bbp->is_free = 0;
        return bbp->ptr;
    }
}


void* bud_realloc(void* ptr, size_t size)
{
    return NULL;
}


void bud_free(void* ptr)
{

}

void bud_show_stats()
{

}


int bud_set_minimum(int min)
{
    if (min <= max_limit)
    {
        min_limit = MAX(0, min);
    }
    return min_limit;
}


int bud_set_maximum(int max)
{
    if (max == -1)
    {
        max_limit = -1;
    } else if (max > min_limit)
    {
        max_limit = MIN(1, max);
    }
    return max_limit;
}